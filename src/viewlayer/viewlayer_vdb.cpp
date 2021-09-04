#include <iostream>
#include <boost/filesystem.hpp>

#include <openvdb/tools/Composite.h>
#include <openvdb/tools/TopologyToLevelSet.h>
#include <openvdb/tools/VelocityFields.h>
#include <openvdb/tools/LevelSetUtil.h>
#include <openvdb/tools/LevelSetAdvect.h>
#include <openvdb/tools/LevelSetFilter.h>
#include <openvdb/tools/Interpolation.h>
#include <openvdb/tools/GridTransformer.h>

#include "SoA.hpp"
#include "application.hpp"
#include "util/util_openvdb.hpp"
#include "resource/OpenvdbResource.hpp"
#include "resource/MeshResource.hpp"
#include "scene_object/MeshObject.hpp"
#include "scene_object/MeshObjectWithData.hpp"
#include "scene_object/LineObject.hpp"
#include "scene_object/PointObject.hpp"
#include "scene_object/Skull.hpp"
#include "scene_object/AreaPicker.hpp"
#include "scene_object/IntersectionPointObject.hpp"
#include "util/util_mesh.hpp"
#include "util/util_color.hpp"
#include "util/util_ui.hpp"
#include "util/util_timer.hpp"
#include "tinyply.hpp"
#include "ray.hpp"

#include "grid_operator.hpp"
#include "grid_operator_filter.hpp"
#include "grid_operator_segment_sdf.hpp"
#include "grid_operator_level_set_filter.hpp"
// #include "grid_operator_border.hpp"

void createGridVisualization(openvdb::FloatGrid::Ptr grid, const std::string& name, float scale = 1.0f)
{
    Timer timer;
    openvdb::FloatGrid::Ptr visualization_grid = openvdb::FloatGrid::create(grid->treePtr());
    openvdb::math::Transform::Ptr t = openvdb::math::Transform::createLinearTransform(scale);
    visualization_grid->setTransform(t);

    std::vector<float> data;
    std::vector<glm::uvec2> indices;
    SoA_Region_List regions = openvdb_create_node_tree_wireframe_visualization(*visualization_grid, data, indices);

    OpenglObjectDescriptor descriptor = {
        ArrayLayout::SoA,
        data.data(),
        data.size() * sizeof(data[0]),
        indices.data(),
        indices.size() * sizeof(indices[0]),
        indices.size() * 2,
        {
            { "position", AttributeLayout::POSITION, 3, GL_FLOAT, sizeof(float), regions.get("position")->offset },
            { "level", 3, 1, GL_UNSIGNED_BYTE, sizeof(GLubyte), regions.get("level")->offset },
        },
        Shader::get("grid_visualization"),
    };

    LineObject* visualization = new LineObject(name, descriptor);
    app.addSceneObject(visualization);
}

PointObject* createVoxelVisualizationWithData(const openvdb::FloatGrid::Ptr grid, const std::string& name, float scale, Shader* shader)
{
    openvdb::FloatGrid::Ptr visualization_grid = openvdb::FloatGrid::create(grid->treePtr());
    openvdb::math::Transform::Ptr t = openvdb::math::Transform::createLinearTransform(scale);
    visualization_grid->setTransform(t);

    std::vector<glm::vec3> points;
    std::vector<float> values;
    openvdb_extract_active_voxel_positions(*visualization_grid, points);
    openvdb_extract_active_voxel_values(*visualization_grid, values);

    std::vector<SoA_Memcpy_Descriptor> soa_descriptor = {
        { { "position", sizeof(points[0]), points.size() }, SoA_Memcpy_Descriptor::ARRAY,  (void*)points.data() },
        { { "probability", sizeof(values[0]), values.size() }, SoA_Memcpy_Descriptor::ARRAY, (void*)values.data() },
    };
    assert(SoA_get_size_bytes(soa_descriptor) % sizeof(float) == 0 && "Remainder was not 0.");
    std::vector<float> data;
    data.resize(SoA_get_size_bytes(soa_descriptor) / sizeof(float));
    SoA_memcpy(data.data(), soa_descriptor);

    SoA_Region_List regions = SoA_create_region_list(soa_descriptor);

    OpenglObjectDescriptor descriptor = {
        ArrayLayout::SoA,
        data.data(),
        data.size() * sizeof(data[0]),
        0,
        0,
        points.size(),
        {
            { "position", AttributeLayout::POSITION, 3, GL_FLOAT, sizeof(float), regions.get("position")->offset },
            { "probability", 3, 1, GL_FLOAT, sizeof(float), regions.get("probability")->offset },
        },
        shader,
        OpenglBufferFlag::VAO | OpenglBufferFlag::VBO,
    };

    PointObject* visualization = new PointObject(name, descriptor);
    app.addSceneObject(visualization);

    return visualization;
}

PointObject* createVoxelVisualizationWithoutData(const openvdb::FloatGrid::Ptr grid, const std::string& name, float scale, Shader* shader)
{
    openvdb::FloatGrid::Ptr visualization_grid = openvdb::FloatGrid::create(grid->treePtr());
    openvdb::math::Transform::Ptr t = openvdb::math::Transform::createLinearTransform(scale);
    visualization_grid->setTransform(t);

    std::vector<glm::vec3> points;
    openvdb_extract_active_voxel_positions(*visualization_grid, points);

    OpenglObjectDescriptor descriptor = {
        ArrayLayout::SoA,
        points.data(),
        points.size() * sizeof(points[0]),
        0,
        0,
        points.size(),
        {
            { "position", AttributeLayout::POSITION, 3, GL_FLOAT, sizeof(float), 0 },
        },
        shader,
        OpenglBufferFlag::VAO | OpenglBufferFlag::VBO,
    };

    PointObject* visualization_object = new PointObject(name, descriptor);
    app.addSceneObject(visualization_object);

    return visualization_object;
}

void setBorderZero(openvdb::Vec3SGrid::Ptr velocity_field, openvdb::math::Transform::Ptr transform, float angle_threshold) {
    const glm::vec3 picker_pos = app.area_picker->getTranslation();
    const float picker_radius = app.area_picker->getRadius();
    openvdb::Vec3d area_picker_pos = transform->worldToIndex(openvdb::Vec3d(picker_pos.x, picker_pos.y, picker_pos.z));
    float distance_scale_index = transform->worldToIndex(openvdb::Vec3d(1, 1, 1)).x();
    for(openvdb::Vec3SGrid::ValueOnIter iter = velocity_field->beginValueOn(); iter; ++iter) {
        iter.setValue(iter.getValue() * app.advection_scale);
        if(!iter.isVoxelValue())
            continue;
        if(!app.user_selection.size())
            continue;
        float d = (iter.getCoord() - area_picker_pos).length();
        if(d > (picker_radius * distance_scale_index))
            continue;

        float distance = std::numeric_limits<float>::max();
        const IntersectionPoint* closest;
        for(const IntersectionPoint& intersection : app.user_selection) {
            float current_distance = (iter.getCoord() - transform->worldToIndex({intersection.point.x, intersection.point.y, intersection.point.z})).length();
            if(current_distance < distance) {
                closest = &intersection;
                distance = current_distance;
            }
        }

        if(distance <= (app.selection_radius * distance_scale_index)) {
            const openvdb::Vec3f normal(closest->normal.x, closest->normal.y, closest->normal.z);
            iter.setValue(iter.getValue() * (iter.getValue().unit().dot(normal) <= angle_threshold));
        }
    }
}

LineObject* createGridNormalsVisualization(const openvdb::FloatGrid::Ptr grid, const std::string& name, float scale, Shader* shader, float angle_threshold)
{
    openvdb::FloatGrid::Ptr visualization_grid = openvdb::FloatGrid::create(grid->treePtr());
    openvdb::math::Transform::Ptr t = openvdb::math::Transform::createLinearTransform(scale);
    visualization_grid->setTransform(t);

    std::vector<glm::vec3> points;
    std::vector<glm::vec3> normals;
    openvdb_extract_active_voxel_positions(*visualization_grid, points);

    // openvdb_extract_active_voxel_normals(*visualization_grid, normals);
    openvdb::Vec3SGrid::Ptr normals_grid = openvdb::tools::gradient(*visualization_grid);
    setBorderZero(normals_grid, t, angle_threshold);
    openvdb::Vec3d normal;
    size_t normal_count = 0;
    normals.resize(normals_grid->activeVoxelCount());
    for(openvdb::Vec3SGrid::TreeType::ValueOnCIter iter = normals_grid->tree().cbeginValueOn(); iter; ++iter, ++normal_count)
    {
        if(!iter.isVoxelValue()) continue;

        normal = iter.getValue();
        normals[normal_count].x = static_cast<float>(normal[0]);
        normals[normal_count].y = static_cast<float>(normal[1]);
        normals[normal_count].z = static_cast<float>(normal[2]);
    }

    assert(points.size() == normals.size() && "Expected points and normals to have same size.");

    std::vector<glm::vec3> data;
    std::vector<glm::uvec2> indices;

    const size_t division_factor = 12;
    const size_t point_count = floor(points.size() / division_factor);
    data.resize(point_count * 2);
    indices.resize(point_count);
    for(int i = 0; i < point_count; ++i) {
        data[2*i] = points[i * division_factor];
        data[2*i + 1] = points[i * division_factor] + 0.1f * normals[i * division_factor];
        indices[i] = glm::uvec2(2*i, 2*i + 1);
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
        shader,
        uint32_t(OpenglBufferFlag::ALL),
    };

    LineObject* visualization_object = new LineObject(name, descriptor);
    app.addSceneObject(visualization_object);

    return visualization_object;
}

void load_grid_and_apply_transformation(const OpenvdbResource& vdb_resource, const SceneObject::Transformation& object_transformation, openvdb::FloatGrid::Ptr& out_grid)
{
    openvdb::FloatGrid::Ptr current_grid;
    vdb_resource.load(&current_grid);

    // https://academysoftwarefoundation.github.io/openvdb/codeExamples.html#sResamplingTools
    openvdb::math::Transform::Ptr transform_1 = current_grid->transformPtr();
    openvdb::math::Transform::Ptr transform_2 = openvdb::math::Transform::createLinearTransform(10); // scale to original 1:1 size in index <-> world
    glm_to_openvdb_transform(object_transformation, *transform_1);
    openvdb::Mat4R transform_mat = transform_1->baseMap()->getAffineMap()->getMat4() * transform_2->baseMap()->getAffineMap()->getMat4();

    openvdb::tools::GridTransformer transformer(transform_mat);
    if(!out_grid) {
        out_grid = openvdb::FloatGrid::create();
        out_grid->setGridClass(current_grid->getGridClass());
    }
    transformer.transformGrid<openvdb::tools::BoxSampler, openvdb::FloatGrid>(*current_grid, *out_grid);
    out_grid->tree().prune();
};

void grid_load_batch_range(const std::vector<SceneObject*>& sources, int start_index, int end_index, std::vector<openvdb::FloatGrid::Ptr>& out_grids)
{
    for(int index = start_index; index <= end_index; ++index)
    {
        SceneObject* scene_object = sources[index];
        MeshResource mesh_resource(scene_object->getName());
        OpenvdbResource vdb_resource(mesh_resource);
        load_grid_and_apply_transformation(vdb_resource, scene_object->getActiveTransformation(), out_grids[index]);
    }
}

void loadTargetGrid(openvdb::FloatGrid::Ptr& target_grid) {
    Skull* scene_object = app.getTargetObject();
    MeshResource mesh_resource(scene_object->getName());
    OpenvdbResource vdb_resource(mesh_resource);
    load_grid_and_apply_transformation(vdb_resource, scene_object->getActiveTransformation(), target_grid);
    std::cout << "Target grid is now loaded." << std::endl;
}

void viewlayer_draw_vdb()
{
    bool exec_load_grids = false;
    bool exec_combine_grids = false;
    bool exec_postprocess_grid = false;

    static int halfWidth = 3;
    static int closingSteps = 1;
    static int dilationSteps = 0;
    static int smoothingSteps = 0;
    static bool subtract_target = false;
    static float threshold = 0.8f;
    static std::vector<openvdb::FloatGrid::Ptr> grids;
    static openvdb::FloatGrid::Ptr probability_grid;
    static openvdb::FloatGrid::Ptr post_processed_grid;
    static openvdb::FloatGrid::Ptr target_grid;

    ImGui::Begin("Reconstruction", NULL);

    ImGui::SliderFloat("Prob. Threshold", &threshold, 0.0f, 1.0f, "%.3f");

    ImGui::BeginDisabled(!app.getTargetObject() || !app.getAlignSources().size());
        if(ImGui::Button("Run Pipeline All")) {
            exec_load_grids = true;
            exec_combine_grids = true;
            exec_postprocess_grid = true;
        }
    ImGui::EndDisabled();

    if(!app.getAlignSources().size()) {
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 118, 33, 255));
        ImGui::TextWrapped("No source skulls available.");
        ImGui::PopStyleColor();
    }

    ImGui::BeginDisabled(!probability_grid);
        if(ImGui::Button("Postprocess & Meshing")) {
            exec_postprocess_grid = true;
        }
        ImGui::Separator();
        if(ImGui::Button("Thresholded Voxels Visualization")) {
            PointObject* object = createVoxelVisualizationWithData(probability_grid, "Thresholded Voxels", 0.1, Shader::get("thresholded_voxel_visualization"));
            object->getPoint()->addUniformLayout({ "u_threshold", UniformType::FLOAT, &threshold });
        }
        if(ImGui::Button("Probability Tree Visualization")) {
            createGridVisualization(probability_grid, "Propability Grid", 0.1);
        }
    ImGui::EndDisabled();

    ImGui::BeginDisabled(!post_processed_grid);
        if(ImGui::Button("Postprocess Tree Visualization")) {
            createGridVisualization(post_processed_grid, "PostProcess Grid", 0.1);
        }
    ImGui::EndDisabled();

    ImGui::Dummy(ImVec2(-FLT_MAX, ImGui::GetTextLineHeight() * 0.2));
    ImGui::Separator();
    ImGui::Dummy(ImVec2(-FLT_MAX, ImGui::GetTextLineHeight() * 0.2));

    ImGui::BeginDisabled(!app.getTargetObject());
        ImGui::Checkbox("Subtract Target", &subtract_target);
        if(!app.getTargetObject()) {
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 118, 33, 255));
            ImGui::TextWrapped("There is no target object set.");
            ImGui::PopStyleColor();
        }
    ImGui::EndDisabled();

    ImGui::Text("Offset Target:");
    ImGui::BeginDisabled(!app.user_selection.size());
        ImGui::DragFloat("Normals Angle Threshold", &app.angle_threshold, 0.01, -1.0f, 1.0f);
        ImGui::DragFloat("User Selection Radius", &app.selection_radius, 0.01, 0.1f, 10.0f, "%.3f");
        ImGui::Checkbox("Show User Selection", &app.show_user_selection);
        ImGui::Checkbox("Show Region Selection", &app.show_region_selection);
    ImGui::EndDisabled();
    if(!app.user_selection.size()) {
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 118, 33, 255));
        ImGui::TextWrapped("No user selection has been added.");
        ImGui::PopStyleColor();
    }

    ImGui::SliderFloat("Offset Scale", &app.advection_scale, 0.1f, 20.0f, "%.3f");
    ImGui::BeginDisabled(!app.getTargetObject());
        if(ImGui::Button("Offset Target Grid")) {
            if(!target_grid)
                loadTargetGrid(target_grid);

            openvdb::Vec3SGrid::Ptr velocity_field = openvdb::tools::gradient(*target_grid);
            openvdb::math::Transform::Ptr t = openvdb::math::Transform::createLinearTransform(0.1);

            setBorderZero(velocity_field, t, app.angle_threshold);

            openvdb::tools::DiscreteField<openvdb::Vec3SGrid, openvdb::tools::BoxSampler> field(*velocity_field);
            openvdb::tools::LevelSetAdvection advection(*target_grid, field);
            advection.advect(0.0, 1.0f);
        }
    ImGui::EndDisabled();

    ImGui::SameLine();

    ImGui::BeginDisabled(!app.getTargetObject());
        ImGui::BeginDisabled(!target_grid);
        if(ImGui::Button("Reset Target Grid")) {
            target_grid = nullptr;
            loadTargetGrid(target_grid);
        }
        ImGui::EndDisabled();

        if(ImGui::Button("Visualize Target Normals")) {
            if(!target_grid)
                loadTargetGrid(target_grid);

            LineObject* object = createGridNormalsVisualization(target_grid, "Target Normals", 0.1, Shader::get("flat_random_line"), app.angle_threshold);
        }

        ImGui::SameLine();

        if(ImGui::Button("Visualize Target Voxels")) {
            if(!target_grid)
                loadTargetGrid(target_grid);

            PointObject* object = createVoxelVisualizationWithoutData(target_grid, "Offset Target Grid", 0.1, Shader::get("flat_random_point"));
        }
    ImGui::EndDisabled();

    ImGui::Dummy(ImVec2(-FLT_MAX, ImGui::GetTextLineHeight() * 0.2));
    ImGui::Separator();
    ImGui::Dummy(ImVec2(-FLT_MAX, ImGui::GetTextLineHeight() * 0.2));

    static std::vector<GridOperator*> grid_operators;
    static bool initalized = false;
    if(!initalized) {
        GridOperatorSegmentSDF* operator_0 = new GridOperatorSegmentSDF();
        GridOperatorFilter* operator_1 = new GridOperatorFilter();
        GridOperatorLevelSetFilter* operator_2 = new GridOperatorLevelSetFilter();
        operator_2->iteration_count = 4;
        GridOperatorSegmentSDF* operator_3 = new GridOperatorSegmentSDF();
        GridOperator* operator_4 = (GridOperator*)new GridOperatorFilter();
        grid_operators.push_back((GridOperator*)operator_0);
        grid_operators.push_back((GridOperator*)operator_1);
        grid_operators.push_back((GridOperator*)operator_2);
        grid_operators.push_back((GridOperator*)operator_3);
        grid_operators.push_back((GridOperator*)operator_4);
        initalized = true;
    }

    for(int i = 0; i < grid_operators.size(); ++i) {
        GridOperator* grid_operator = grid_operators[i];

        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        ImGui::PushID(i);
        ImVec2 start_cursor_pos = ImGui::GetCursorPos();
        ImVec2 start_cursor_pos_screen = ImGui::GetCursorScreenPos();

        ImGui::SetCursorPosX(start_cursor_pos.x + 15);
        ImGui::SetCursorPosY(start_cursor_pos.y + 15);
        ImGui::BeginGroup();
        float width = ImGui::GetContentRegionAvail().x + 15;
        draw_list->PushClipRect(start_cursor_pos_screen, {start_cursor_pos_screen.x + width, start_cursor_pos_screen.y + 10000}, true);
        ImGui::Text(grid_operator_type_names[grid_operator->type]);
        grid_operator->drawUI();
        if(ImGui::Button("Move up")) {
            int n_next = i - 1;
            if (n_next >= 0 && n_next < grid_operators.size()) {
                grid_operators[i] = grid_operators[n_next];
                grid_operators[n_next] = grid_operator;
            }
        }
        ImGui::SameLine();
        if(ImGui::Button("Move down")) {
            int n_next = i + 1;
            if (n_next >= 0 && n_next < grid_operators.size()) {
                grid_operators[i] = grid_operators[n_next];
                grid_operators[n_next] = grid_operator;
            }
        }
        ImGui::SameLine();
        if(ImGui::Button("Remove")) {
            grid_operators.erase(grid_operators.begin() + i);
            i--;
        }
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5);
        ImVec2 end_cursor_pos = ImGui::GetCursorPos();
        ImVec2 end_cursor_pos_screen = ImGui::GetCursorScreenPos();
        ImGui::EndGroup();
        ImGui::PopClipRect();

        draw_list->AddRect(start_cursor_pos_screen, {start_cursor_pos_screen.x + width, end_cursor_pos_screen.y}, ImGui::GetColorU32(ImGuiCol_Border), 2);

        ImGui::PopID();
    }

    static int selected_operator_type = GridOperator_LevelSetFilter;
    if(ImGui::BeginCombo("##add_operator", grid_operator_type_names[selected_operator_type])) {
        for(int32_t i = 0; i < GridOperator_MAX; ++i) {
            bool is_selected = selected_operator_type == i;
            if(ImGui::Selectable(grid_operator_type_names[i], is_selected)) {
                selected_operator_type = i;
            }
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    ImGui::SameLine();
    if(ImGui::Button("Add")) {
        GridOperator* grid_operator;
        if(selected_operator_type == GridOperator_SegmentSDF) {
            grid_operator = new GridOperatorSegmentSDF();
        } else if(selected_operator_type == GridOperator_Filter) {
            grid_operator = new GridOperatorFilter();
        } else if(selected_operator_type == GridOperator_LevelSetFilter) {
            grid_operator = new GridOperatorLevelSetFilter();
        // } else if(selected_operator_type == GridOperator_Border) {
        //     grid_operator = new GridOperatorBorder();
        } else {
            std::cout << "Unsupported operator type" << std::endl;
            exit(-1);
        }
        grid_operators.push_back(grid_operator);
    }

    ImGui::Dummy(ImVec2(-FLT_MAX, ImGui::GetTextLineHeight() * 0.2));
    ImGui::Separator();
    ImGui::Dummy(ImVec2(-FLT_MAX, ImGui::GetTextLineHeight() * 0.2));

    ImGui::Text("Meshing:");
    ImGui::DragInt("Band Half Width", &halfWidth, 1, 0, 10);
    ImGui::DragInt("Closing Steps", &closingSteps, 1, 0, 10);
    ImGui::DragInt("Dilation Steps", &dilationSteps, 1, 0, 10);
    ImGui::DragInt("Smoothing Steps", &smoothingSteps, 1, 0, 10);

    SceneObject* selection = app.getSelectedObject();
    ImGui::BeginDisabled(!selection || selection->getType() != SceneObject::MESH_OBJECT_WITH_DATA);
        if(ImGui::Button("Export Selected Implant"))
        {
            MeshObjectWithData* mesh = (MeshObjectWithData*)selection;
            boost::filesystem::path default_path = Resource::getDataRootDirectory() / "export";
            if(!boost::filesystem::exists(default_path)) {
                if(!boost::filesystem::create_directories(default_path)) {
                    std::cout << "Error creating directories: \"" << default_path.string() << "\"" << std::endl;
                }
            }
            boost::filesystem::path path;
            nfdfilteritem_t filterItem[1] = {{"PLY", "ply"}};
            if(ui_file_dialog_save(path, default_path.string(), mesh->getName() + ".ply", filterItem, 1) == NFD_OKAY)
                write_ply(path.string(), mesh->getTriangles(), mesh->getPoints());
        }
    ImGui::EndDisabled();

    ImGui::End();

    Timer timer_total_pipeline;
    if(exec_load_grids) {
        std::vector<SceneObject*> sources = app.getAlignSources();

        grids.clear();
        grids.resize(sources.size());
        for(auto& grid_ptr : grids)
            grid_ptr = openvdb::FloatGrid::create();

        if(grids.size()) {
            Timer timer;

            std::vector<std::thread> threads;
            const int thread_count = 8;
            const int grids_per_thread = std::max(int(sources.size() / float(thread_count) + 0.5f), 3);
            for(int index = 0; index < thread_count; ++index)
            {
                const int start = index * grids_per_thread;
                if(start >= sources.size()) continue;
                int end = (start + grids_per_thread) - 1;
                if(end >= sources.size()) end = sources.size() - 1;
                threads.emplace_back(grid_load_batch_range, sources, start, end, grids);
            }

            for(auto& thread : threads)
                thread.join();

            std::cout << "Load All Grids: " << timer.stop().toString() << std::endl;
        }
        else {
            std::cout << "Finished: No input grids given." << std::endl;
        }
    }

    if(exec_combine_grids) {
        Timer timer;
        probability_grid = openvdb::FloatGrid::create(0.0);
        openvdb::FloatGrid::Accessor probability_accessor = probability_grid->getAccessor();
        for(auto grid : grids)
        {
            for(openvdb::FloatGrid::ValueAllIter iter = grid->beginValueAll(); iter; ++iter)
            {
                if(*iter > 0.0f) continue; // voxel or tile is entirely outside // TODO: check if active state matters for tile values

                if(iter.isVoxelValue())
                {
                    const openvdb::Coord coordiante = iter.getCoord();
                    const bool is_active = probability_accessor.isValueOn(coordiante);
                    const float value = is_active * probability_accessor.getValue(coordiante); // make it 0, if no value is set yet
                    probability_accessor.setValue(coordiante, value + 1);
                }
                else
                {
                    openvdb::CoordBBox bbox = iter.getBoundingBox();
                    for(openvdb::CoordBBox::XYZIterator bbox_iter = bbox.beginXYZ(); bbox_iter; ++bbox_iter)
                    {
                        const openvdb::Coord& coordiante = *bbox_iter;
                        const bool is_active = probability_accessor.isValueOn(coordiante);
                        const float value = is_active * probability_accessor.getValue(coordiante); // make it 0, if no value is set yet
                        probability_accessor.setValue(coordiante, value + 1);
                    }
                }
            }
        }

        const size_t grid_count = grids.size();
        for(openvdb::FloatGrid::ValueOnIter iter = probability_grid->beginValueOn(); iter; ++iter)
        {
            float count = *iter;
            float active_ratio = count / grid_count;
            const openvdb::Coord coordiante = iter.getCoord();
            probability_accessor.setValue(coordiante, active_ratio);
        }

        probability_grid->tree().prune();

        std::cout << "Combine grids: " << timer.stop().toString() << std::endl;
    }

    if(exec_postprocess_grid) {
        Timer timer;
        const size_t grid_count = grids.size();
        post_processed_grid = openvdb::FloatGrid::create(1.0);
        openvdb::FloatGrid::Accessor post_process_accessor = post_processed_grid->getAccessor();
        if(subtract_target) {
            if(!target_grid) {
                loadTargetGrid(target_grid);
            }
            openvdb::FloatGrid::ConstUnsafeAccessor target_accessor = target_grid->getConstUnsafeAccessor();
            for(openvdb::FloatGrid::ValueOnCIter iter = probability_grid->cbeginValueOn(); iter; ++iter) {
                if(*iter >= threshold && target_accessor.getValue(iter.getCoord()) >= 0)
                    post_process_accessor.setValueOn(iter.getCoord(), *iter);
            }
        }
        else {
            for(openvdb::FloatGrid::ValueOnCIter iter = probability_grid->cbeginValueOn(); iter; ++iter) {
                if(*iter >= threshold) post_process_accessor.setValueOn(iter.getCoord(), *iter);
            }
        }

        std::cout << "Grid Postprocessing: " << timer.stop().toString() << std::endl;

        // Meshing
        timer.start();
        openvdb::FloatGrid::Ptr meshing_grid = openvdb::tools::topologyToLevelSet(*post_processed_grid, halfWidth, closingSteps, dilationSteps, smoothingSteps);
        openvdb::math::Transform::Ptr t = openvdb::math::Transform::createLinearTransform(0.1); // TODO: maybe we can get rid of this step
        meshing_grid->setTransform(t);

        static std::vector<openvdb::FloatGrid::Ptr> in_grids;
        in_grids = {meshing_grid};
        std::vector<openvdb::FloatGrid::Ptr> results = grid_operator_execute_all(grid_operators, in_grids);

        for(auto& grid : results)
        {
            std::vector<glm::vec3> mesh_points;
            std::vector<glm::uvec3> mesh_indices;
            MeshResource::gridToMeshData(grid, mesh_points, mesh_indices);

            std::vector<float> data;
            std::vector<glm::uvec3> indices;
            SoA_Region_List regions = SoA_mesh_from_position_with_flat_normal(mesh_points, mesh_indices, data, indices);

            OpenglObjectDescriptor descriptor = {
                ArrayLayout::SoA,
                data.data(),
                data.size() * sizeof(data[0]),
                indices.data(),
                indices.size() * sizeof(indices[0]),
                indices.size() * 3,
                {
                    { "position", AttributeLayout::POSITION, 3, GL_FLOAT, sizeof(float), regions.get("position")->offset },
                    { "normal", AttributeLayout::NORMAL, 3, GL_FLOAT, sizeof(float), regions.get("normal")->offset },
                },
                Shader::get("3D_position_normal"),
            };

            MeshObjectWithData* mesh_object = new MeshObjectWithData("implant_" + std::to_string(threshold), descriptor);
            mesh_object->setData(mesh_points, mesh_indices);
            mesh_object->color_ = glm::vec4(0, 1, 1, 1);
            mesh_object->getMesh()->addUniformLayout({ "color", UniformType::VEC4, (void*)&mesh_object->color_ });
            app.addSceneObject(mesh_object);
        }

        std::cout << "Meshing took: " << timer.stop().toString() << std::endl;
    }

    if(exec_load_grids || exec_combine_grids || exec_postprocess_grid)
        std::cout << "Pipline took " << timer_total_pipeline.stop().toString() << std::endl;
}