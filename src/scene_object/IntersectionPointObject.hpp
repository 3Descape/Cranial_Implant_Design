#include <glm/gtx/matrix_cross_product.hpp>

#include "opengl_object/Mesh.hpp"
#include "resource/Resource.hpp"
#include "scene_object/SceneObject.hpp"
#include "util/util_mesh.hpp"
#include "tinyply.hpp"

#ifndef INTERSECTION_POINT_OBJECT_H
#define INTERSECTION_POINT_OBJECT_H

class IntersectionPointObject : public SceneObject
{
    typedef enum Status {
        UNINITALIZED,
        INITALIZED,
        DESTROYED,
    } Status;
    private:
        inline static Mesh* mesh_ = nullptr;
        inline static Status status_ = UNINITALIZED;
        bool highlighted_ = false;

    public:
        IntersectionPointObject(const std::string& name = "") : SceneObject(name, mesh_, Type::UNSPECIFIED)
        {
            if(status_ == UNINITALIZED)
                createInstance();
        }

        static void createInstance()
        {
            boost::filesystem::path file = Resource::getDataRootDirectory() / boost::filesystem::path("application/intersection.ply");

            std::vector<float> data;
            std::vector<glm::uvec3> triangles;
            std::vector<glm::vec3> points;
            std::vector<glm::vec4> colors;
            std::vector<glm::vec3> normals;
            read_blender_ply(file.string(), points, triangles, colors, normals);
            SoA_Region_List regions = SoA_mesh_from_position_color4_normal(points, colors, normals, data);

            OpenglObjectDescriptor descriptor = {
                ArrayLayout::SoA,
                data.data(), // buffer data
                data.size() * sizeof(data[0]), // buffer size
                triangles.data(), // buffer data
                triangles.size() * sizeof(triangles[0]), // size of buffer
                triangles.size() * 3, // indices count
                {
                    { "position", AttributeLayout::POSITION, 3, GL_FLOAT, sizeof(float), regions.get("position")->offset },
                    { "color", AttributeLayout::COLOR, 4, GL_FLOAT, sizeof(float), regions.get("color")->offset },
                    { "normal", AttributeLayout::NORMAL, 3, GL_FLOAT, sizeof(float), regions.get("normal")->offset },
                },
                Shader::get("intersectionpoint"),
            };

            const std::vector<UniformLayout> UniformLayout = {
                {"model", UniformType::MAT4, nullptr},
                {"view", UniformType::MAT4, nullptr},
                {"projection", UniformType::MAT4, nullptr},
                {"u_highlighted", UniformType::BOOL, nullptr},
            };

            mesh_ = new Mesh(descriptor, UniformLayout);
            status_ = INITALIZED;
        }

        static void destroyInstance()
        {
            delete mesh_;
            mesh_ = nullptr;
            status_ = DESTROYED;
        }

        void setHighlight(bool status)
        {
            highlighted_ = status;
        }

        virtual void draw(const glm::mat4& view, const glm::mat4& projection) override
        {
            updateModelMatrix();
            Shader* shader = mesh_->getShader();
            shader->use();

            // set sphere scaling via a matrix
            mesh_->updateUniforms({
                {"model", { (void*)&model_, 1 }},
                {"view", { (void*)&view, 1 }},
                {"projection", { (void*)&projection, 1 }},
                {"u_highlighted", { (void*)&highlighted_, 1 }},
            });

            mesh_->draw();
        }

        virtual void updateModelMatrix()
        {
            glm::vec3 a(0, 0, 1);
            glm::vec3 b = getRotation(); // = normal vector of intersection point
            glm::vec3 v = glm::cross(a, b);
            float s = glm::length(v);
            float c = glm::dot(a, b);
            glm::mat3 v_x = glm::matrixCross3(v);

            const Transformation& t = getActiveTransformation();
            glm::mat4 scale = glm::scale(glm::mat4(1.0), glm::vec3(0.3));
            glm::mat4 rotation = glm::mat4(glm::mat3(1.0) + v_x + v_x * v_x * (1.0f / (1.0f + c)));
            glm::mat4 translation = glm::translate(glm::mat4(1.0), t.translation);

            model_ = xyzToxzy(translation * rotation * scale);
        }
};
#endif // INTERSECTION_POINT_OBJECT_H