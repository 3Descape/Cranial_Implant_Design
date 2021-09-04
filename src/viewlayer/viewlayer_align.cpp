#include "application.hpp"
#include "resource/NrrdResource.hpp"
#include "resource/DescriptorResource.hpp"
#include "resource/MeshResource.hpp"
#include "resource/DescriptorResource.hpp"
#include "descriptor/GASDDescriptor.hpp"
#include "scene_object/SceneObject.hpp"
#include "scene_object/AreaPicker.hpp"
#include "scene_object/Skull.hpp"
#include "UISettings.hpp"
#include "util/util_mesh.hpp"
#include "align.hpp"

void viewlayer_draw_align()
{
    ICPSettings& icp_settings = app.icp_settings;

    ImGui::Begin("Alignment");

    ImGui::Separator();
    if(ImGui::BeginCombo("Clipping", icp_settings.icp_type.getLabel()))
    {
        for(ICPType::Type entry : ICPType::elements)
        {
            const bool is_selected = entry == icp_settings.icp_type.value;
            if (ImGui::Selectable(ICPType::getLabel(entry), is_selected)) {
                icp_settings.icp_type = entry;
                if(entry == ICPType::LLS_CLIPPED_AREAPICKER)
                    app.show_region_selection = true;
                else if(entry == ICPType::LLS_CLIPPED_SELECTION) {
                    app.show_region_selection = true;
                    app.show_user_selection = true;
                }
                else if(entry == ICPType::LLS) {
                    app.show_region_selection = false;
                    app.show_user_selection = false;
                }
            }

            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }

        ImGui::EndCombo();
    }

    ImGui::BeginDisabled(!(icp_settings.icp_type == ICPType::LLS_CLIPPED_SELECTION));
        ImGui::BeginDisabled(!app.user_selection.size());
        ImGui::DragFloat("User Selection Radius", &app.selection_radius, 0.01, 0.1f, 10.0f, "%.3f");
        ImGui::EndDisabled();

        if(ImGui::Button("Clear Selection")) {
            app.user_selection.clear();
        }
    ImGui::EndDisabled();

    ImGui::Checkbox("Show User Selection", &app.show_user_selection);
    ImGui::Checkbox("Show Region Selection", &app.show_region_selection);

    ImGui::Separator();

    ImGui::Checkbox("Use Symmetric Objective", &icp_settings.use_symmetric_objective);
    ImGui::BeginDisabled(!icp_settings.use_symmetric_objective);
    ImGui::Checkbox("Use Same Normal Direction", &icp_settings.use_same_normal_direction);
    ImGui::EndDisabled();

    ImGui::Checkbox("Use Reciprocal Correspondences", &icp_settings.use_reciprocal_correspondences);
    ImGui::Checkbox("Switch Align Order(Target->Source)", &icp_settings.swap_source_and_target);
    ImGui::Checkbox("Reset Transform Before Alignment", &icp_settings.reset_transform);
    ImGui::DragInt("Max Iteration Count", &icp_settings.max_iterations_count);
    ImGui::DragFloat("Transform Epsilon", &icp_settings.transformation_epsilon, 0.01, 0.0f, 1.0f, "%.5f");
    ImGui::DragFloat("Normal Rejector Threshold", &icp_settings.normal_rejector_threshold, 0.01, -1.0f, 1.0f, "%.3f");
    ImGui::Checkbox("With Correspondence Visualization", &icp_settings.with_correspondence_vis);
    ImGui::Checkbox("With Source Visualization", &icp_settings.with_source_vis);
    ImGui::Checkbox("With Target Visualization", &icp_settings.with_target_vis);

    const int skull_count = app.getSkullCount();
    ImGui::BeginDisabled(!skull_count);
    {
        ImGui::Text("Target:");
        Skull* target_object = app.getTargetObject();

        if(ImGui::BeginCombo("##target", target_object ? target_object->getName().c_str()  : "Invalid Target"))
        {
            for(int32_t i = 0; i < app.scene_objects.size(); ++i)
            {
                if(app.scene_objects.at(i)->getType() != SceneObject::SKULL) continue;

                bool is_selected = (i == app.target_index);
                if (ImGui::Selectable(app.scene_objects.at(i)->getName().c_str(), is_selected))
                {
                    app.target_index = i;
                    is_selected = true;
                }

                // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        ImGui::EndDisabled();
    }

    ImGui::Separator();

    bool is_valid_target = app.isValidAlignTarget();
    bool is_valid_source = app.selectedObjectIsValidAlignSource();
    bool is_valid = is_valid_target && is_valid_source;

    {
        ImGui::BeginDisabled(!is_valid_target);

        static int add_similar_count = 20;
        ImGui::DragInt("##similar_count", &add_similar_count, 1.0f, 1, 500);
        ImGui::SameLine();

        if (ImGui::Button("Add Similar Skulls") && is_valid_target)
        {
            Skull* target = app.getTargetObject();

            std::vector<glm::vec3> normals;
            mesh_compute_smooth_normals(target->getPoints(), target->getTriangles(), normals);

            GASDDescriptor descriptor;
            DescriptorResource::computeDescriptor(descriptor, target->getPoints(), normals);
            std::vector<MeshResource> matches;
            DescriptorResource::findMatches(descriptor, add_similar_count, matches);
            app.addMeshObjects(matches);
        }

        ImGui::EndDisabled();
    }

    if(!is_valid || !skull_count)
    {
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 118, 33, 255));
        if(app.getSkullCount() == 0)
            ImGui::TextWrapped("Target: There is no skull present in the scene.");
        ImGui::PopStyleColor();
    }

    if (ImGui::Button("Add all healty skulls"))
    {
        std::vector<boost::filesystem::path> files;
        for(boost::filesystem::directory_iterator itr(NrrdResource::getRootDirectory() / "complete_skull"); itr != boost::filesystem::directory_iterator{}; ++itr)
        {
            // skip directories
            if (!boost::filesystem::is_regular_file(itr->path())) continue;
            files.push_back(itr->path());
        }

        app.addNrrdObjects(files);
    }
    bool is_user_selection_valid = icp_settings.icp_type != ICPType::LLS_CLIPPED_SELECTION || app.user_selection.size() > 0;

    {
        ImGui::BeginDisabled(!is_valid || !is_user_selection_valid);
        // ImGui::SameLine(0, 20);

        if (ImGui::Button("Align") && is_valid) {
            SceneObject* source_object = app.scene_objects[app.selected_object_index];
            std::vector<SceneObject*> new_objects;
            align(*app.getTargetObject(), *(Skull*)source_object, app.icp_settings, new_objects);
            for(auto object : new_objects) {
                app.addSceneObject(object);
            }
        }

        ImGui::EndDisabled();
    }

    ImGui::SameLine();

    std::vector<SceneObject*> align_sources = app.getVisibleAlignSources();
    bool is_valid_multiple = is_valid_target && align_sources.size();

    ImGui::BeginDisabled(!is_valid_multiple || !is_user_selection_valid);
    {
        if(ImGui::Button("Align All Visible") && is_valid_multiple)
        {
            Timer timer;

            std::vector<std::vector<SceneObject*>> new_objects(align_sources.size());
            auto align_work = [&align_sources, &new_objects](int start_index, int end_index) {
                for(int i = start_index; i <= end_index; ++i) {
                    SceneObject* source_object = align_sources[i];
                    align(*app.getTargetObject(), *(Skull*)source_object, app.icp_settings, new_objects[i]);
                }
            };

            std::vector<std::thread> threads;
            const int thread_count = 5;
            const int elements_per_thread = std::ceil(align_sources.size() / float(thread_count));
            for(int i = 0; i < align_sources.size(); ++i)
                threads.emplace_back(align_work, i * elements_per_thread, std::min((i + 1) * elements_per_thread - 1, (int)align_sources.size() - 1));

            for(auto& thread : threads)
                thread.join();

            std::cout << "Alignment took: " << timer.stop().toString() << std::endl;
            for(auto& vector : new_objects)
                for(auto object : vector)
                    app.addSceneObject(object);
        }
        ImGui::EndDisabled();
    }

    if(!is_valid || !skull_count || !is_user_selection_valid)
    {
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 118, 33, 255));
        if(app.getSkullCount() < 2)
            ImGui::TextWrapped("Align: You need at least two skulls for alignment.");
        else if(app.selected_object_index == -1)
            ImGui::TextWrapped("Align: No object selected.");
        else if(app.scene_objects[app.selected_object_index]->getType() != SceneObject::SKULL)
            ImGui::TextWrapped("Align: Selected object must be a skull.");
        if(app.getSkullCount() >= 2 && app.target_index == app.selected_object_index)
            ImGui::TextWrapped("Align: Selected object must be != target.");
        if(!is_user_selection_valid)
            ImGui::TextWrapped("Clipping: You must add at least one selection point for this clipping mode(LMB + Click on target).");
        ImGui::PopStyleColor();
    }

    if(!is_valid_multiple)
    {
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 118, 33, 255));
        if(align_sources.size() == 0)
            ImGui::TextWrapped("AlignAllVisible: No other skulls != the target are visible.");
        ImGui::PopStyleColor();
    }

    ImGui::End();
}