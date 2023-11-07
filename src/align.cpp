#include "align.hpp"

#include <iostream>
#include <string>

#include <pcl/console/print.h>
#include <pcl/point_types.h>
#include <pcl/point_cloud.h>
#include <pcl/common/eigen.h>
#include <pcl/registration/correspondence_rejection_surface_normal.h>
#include "ICP.h"

#include <boost/filesystem.hpp>

#include "scene_object/SceneObject.hpp"
#include "scene_object/Skull.hpp"
#include "scene_object/PointObject.hpp"
#include "scene_object/LineObject.hpp"
#include "scene_object/SequenceLineObject.hpp"
#include "UISettings.hpp"
#include "util/util_opengl.hpp"
#include "util/util_timer.hpp"
#include "tinyply.hpp"
#include "opengl_object/Line.hpp"
#include "opengl_object/Point.hpp"
#include "util/util_mesh.hpp"

typedef pcl::PointNormal PointT;
typedef pcl::PointCloud<PointT> PointCloudT;

void print4x4Matrix(const Eigen::Matrix4f& matrix)
{
  printf ("Rotation matrix(radians):\n");
  printf ("    | %6.3f %6.3f %6.3f | \n", matrix (0, 0), matrix (0, 1), matrix (0, 2));
  printf ("R = | %6.3f %6.3f %6.3f | \n", matrix (1, 0), matrix (1, 1), matrix (1, 2));
  printf ("    | %6.3f %6.3f %6.3f | \n", matrix (2, 0), matrix (2, 1), matrix (2, 2));
  printf ("Translation vector :\n");
  printf ("t = < %6.3f, %6.3f, %6.3f >\n\n", matrix (0, 3), matrix (1, 3), matrix (2, 3));
}

Eigen::Matrix4f create_rotation_matrix(const glm::vec3& rotation, const glm::vec3& translation)
{
    Eigen::Affine3f rx = Eigen::Affine3f(Eigen::AngleAxisf(rotation.x, Eigen::Vector3f(1, 0, 0)));
    Eigen::Affine3f ry = Eigen::Affine3f(Eigen::AngleAxisf(rotation.y, Eigen::Vector3f(0, 1, 0)));
    Eigen::Affine3f rz = Eigen::Affine3f(Eigen::AngleAxisf(rotation.z, Eigen::Vector3f(0, 0, 1)));
    Eigen::Affine3f r = rz * ry * rx;

    Eigen::Affine3f t(Eigen::Translation3f(Eigen::Vector3f(translation.x, translation.y, translation.z)));

    return (t * r).matrix();
}

SceneObject* createCloudVisualization(const PointCloudT::Ptr cloud, const std::string& name)
{
    std::vector<glm::vec3> data;
    data.reserve(cloud->points.size());

    for(auto& p : cloud->points)
        data.push_back(glm::vec3(p.x, p.y, p.z));

    OpenglObjectDescriptor descriptor = {
        ArrayLayout::SoA,
        data.data(),
        data.size() * sizeof(data[0]),
        0,
        0,
        data.size() * 3,
        {
            { "position", AttributeLayout::POSITION, 3, GL_FLOAT, sizeof(float), 0 },
        },
        Shader::get("3D_flat"),
        OpenglBufferFlag::VAO | OpenglBufferFlag::VBO,
    };

    PointObject* scene_object = new PointObject(name, descriptor);
    scene_object->color_ = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
    scene_object->getPoint()->addUniformLayout({"u_color", UniformType::VEC4, &scene_object->color_});

    return scene_object;
}

void addCorrespondenceTimeStep(SequenceLineObject* sequence, const PointCloudT::Ptr source_cloud, const PointCloudT::Ptr target_cloud, const pcl::CorrespondencesPtr correspondences)
{
    if(!correspondences || !correspondences->size()) return;

    std::vector<glm::vec3> data;
    std::vector<glm::uvec2> indices;
    data.reserve(correspondences->size() * 2);
    indices.reserve(correspondences->size());

    uint32_t index = 0;
    for(auto& c : *correspondences)
    {
        if(c.index_match == -1) continue;
        auto& p1 = source_cloud->points.at(c.index_query);
        auto& p2 = target_cloud->points.at(c.index_match);
        data.push_back(glm::vec3(p1.x, p1.y, p1.z));
        data.push_back(glm::vec3(p2.x, p2.y, p2.z));
        indices.push_back(glm::uvec2(index++, index++));
    }

    OpenglObjectDescriptor descriptor = {
        ArrayLayout::SoA,
        data.data(),
        data.size() * sizeof(data[0]),
        indices.data(),
        indices.size() * sizeof(indices[0]),
        indices.size() * 2,
        {
            { "position", AttributeLayout::POSITION, 3, GL_FLOAT, sizeof(float), 0 },
        },
        Shader::get("3D_flat"),
    };

    Line* line = sequence->addTimeStep(descriptor, UniformLayoutDefault);
    line->addUniformLayout({"u_color", UniformType::VEC4, &sequence->color_});
}

void pcl_insert_masked_points(PointCloudT::Ptr cloud, Skull& skull)
{
    auto points = skull.getPoints();
    std::vector<glm::vec3> normals;
    mesh_compute_smooth_normals(points, skull.getTriangles(), normals);
    auto mask = skull.getMaskReference();

    cloud->points.reserve(skull.getMaskActiveCount());

    for(size_t i = 0; i < points.size(); ++i)
    {
        if(!mask[i])
            continue;

        cloud->points.push_back(PointT(points[i].x, points[i].y, points[i].z, normals[i].x, normals[i].y, normals[i].z));
    }
}

void pcl_insert_points(PointCloudT::Ptr cloud, Skull& skull)
{
    auto points = skull.getPoints();
    std::vector<glm::vec3> normals;
    mesh_compute_smooth_normals(points, skull.getTriangles(), normals);

    cloud->points.reserve(points.size());

    for(size_t i = 0; i < points.size(); ++i)
        cloud->points.push_back(PointT(points[i].x, points[i].y, points[i].z, normals[i].x, normals[i].y, normals[i].z));
}

int align(Skull& target, Skull& source, const ICPSettings& settings, std::vector<SceneObject*>& output_objects)
{
    Timer timer;
    bool swapped = settings.swap_source_and_target;

    PointCloudT::Ptr target_cloud(new PointCloudT);
    PointCloudT::Ptr source_cloud(new PointCloudT);

    switch (settings.icp_type.value)
    {
    case ICPType::LLS_CLIPPED_AREAPICKER: {
        target.updateSelectionMaskWithAreaPicker();
        source.updateSelectionMaskWithAreaPicker();

        pcl_insert_masked_points(target_cloud, target);
        pcl_insert_masked_points(source_cloud, source);
    } break;
    case ICPType::LLS_CLIPPED_SELECTION: {
        target.updateSelectionMaskWithUserSelection();
        source.updateSelectionMaskWithAreaPicker();

        pcl_insert_masked_points(target_cloud, target);
        pcl_insert_masked_points(source_cloud, source);
    } break;
    case ICPType::LLS: {
        pcl_insert_points(target_cloud, target);
        pcl_insert_points(source_cloud, source);
    } break;
    default:
        break;
    }

    // The Iterative Closest Point algorithm
    ICP<PointT, PointT> icp;

    pcl::registration::CorrespondenceRejectorSurfaceNormal::Ptr normal_rejector(new pcl::registration::CorrespondenceRejectorSurfaceNormal);
    normal_rejector->setThreshold(settings.normal_rejector_threshold);
    icp.addCorrespondenceRejector(normal_rejector);

    icp.setUseReciprocalCorrespondences(settings.use_reciprocal_correspondences);
    icp.setTransformationEpsilon(settings.transformation_epsilon);
    icp.setMaximumIterations(settings.max_iterations_count);
    if(swapped) {
        icp.setInputSource(target_cloud);
        icp.setInputTarget(source_cloud);
    } else {
        icp.setInputSource(source_cloud);
        icp.setInputTarget(target_cloud);
    }
    Eigen::Matrix4f initial_transform = Eigen::Matrix4f::Identity();

    if(!settings.reset_transform)
        initial_transform = create_rotation_matrix(glm::radians(source.getRotation()), source.getTranslation());

    if(settings.with_correspondence_vis)
    {
        SequenceLineObject* sequence = new SequenceLineObject("Correspondence Visualization");
        sequence->makeLink(&source);

        PointCloudT::Ptr source_cloud_copy(new PointCloudT);
        pcl::copyPointCloud(*source_cloud, *source_cloud_copy);

        PointCloudT::Ptr target_cloud_copy(new PointCloudT);
        pcl::copyPointCloud(*target_cloud, *target_cloud_copy);

        std::function<ICP<PointT, PointT, float>::UpdateVisualizerCallbackSignature> callback =
            [sequence, source_cloud, target_cloud, source_cloud_copy, target_cloud_copy, &icp, swapped](
                size_t iteration,
                const ICP<PointT, PointT, float>::Matrix4& delta_transform,
                const ICP<PointT, PointT, float>::Matrix4& final_transform,
                const pcl::CorrespondencesPtr& correspondences,
                const float score)
        {
            std::cout << "Callback, iteration: " << iteration << std::endl;

            // TODO: refactor since duplicate code(see below)
            Eigen::Transform<float, 3, Eigen::Affine> transformation(final_transform);
            glm::vec3 rotation_radians;
            glm::vec3 translation;
            pcl::getTranslationAndEulerAngles(transformation, translation.x, translation.y, translation.z, rotation_radians.x, rotation_radians.y, rotation_radians.z);

            glm::vec3 scale(1.0f);
            glm::vec3 rotation = glm::degrees(rotation_radians);

            PointCloudT::Ptr transformed_cloud(new PointCloudT);
            const std::string name = (std::string("Iteration ") + std::to_string(iteration));
            if(swapped) {
                sequence->transformations_.push_back({ -translation, -rotation, scale, name, score });
                pcl::copyPointCloud(*source_cloud_copy, *transformed_cloud);
                icp.transformCloud(*transformed_cloud, *transformed_cloud, final_transform.inverse());
                addCorrespondenceTimeStep(sequence, target_cloud, transformed_cloud, correspondences);
            } else {
                sequence->transformations_.push_back({ translation, rotation, scale, name, score });
                pcl::copyPointCloud(*source_cloud_copy, *transformed_cloud);
                icp.transformCloud(*transformed_cloud, *transformed_cloud, final_transform);
                addCorrespondenceTimeStep(sequence, transformed_cloud, target_cloud, correspondences);
            }
        };

        icp.registerVisualizationCallback(callback);

        output_objects.push_back(sequence);
    }

    if(swapped)
        icp.align(*target_cloud, initial_transform);
    else
        icp.align(*source_cloud, initial_transform);

    if(settings.with_target_vis)
        output_objects.push_back(createCloudVisualization(target_cloud, "Target Cloud"));

    if(settings.with_source_vis)
        output_objects.push_back(createCloudVisualization(source_cloud, "Source Cloud"));

    if (!icp.hasConverged()) {
        PCL_ERROR("ICP has not converged.\n");
        return -1;
    }

    std::cout << "Applied ICP in " << timer.stop().toString() << std::endl;
    Eigen::Matrix4f transformation_matrix = icp.getFinalTransformation().cast<float>();
    Eigen::Transform<float, 3, Eigen::Affine> transformation(transformation_matrix);
    glm::vec3 rotation_radians;
    glm::vec3 translation;
    pcl::getTranslationAndEulerAngles(transformation, translation.x, translation.y, translation.z, rotation_radians.x, rotation_radians.y, rotation_radians.z);

    glm::vec3 scale(1.0f);
    const float icp_score = icp.getFitnessScore();
    glm::vec3 rotation = glm::degrees(rotation_radians);
    std::string name = (std::string("Alignment ") + std::to_string(source.transformations_.size()));

    if(swapped) source.transformations_.push_back({ -translation, -rotation, scale, name, icp_score });
    else        source.transformations_.push_back({ translation, rotation, scale, name, icp_score });
    source.selected_transformation_ = source.transformations_.size() - 1;

    return 0;
}