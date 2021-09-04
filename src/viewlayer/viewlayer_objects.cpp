#include <utility>
#include <boost/filesystem.hpp>

#include "util/util_ui.hpp"
#include "application.hpp"
#include "scene_object/AreaPicker.hpp"
#include "scene_object/SceneObject.hpp"
#include "scene_object/SequenceLineObject.hpp"
#include "resource/NrrdResource.hpp"
#include "util/util_timer.hpp"

void viewlayer_draw_objects()
{
    ImGui::Begin("Objects", NULL);

    ImGuiIO& io = ImGui::GetIO();
    ImGuiStyle& style = ImGui::GetStyle();
    static float select_size = 150.0f;
    if(ImGui::BeginListBox("Objects", ImVec2(-FLT_MIN, select_size)))
    {
        if(ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows) && ImGui::IsKeyReleased(ImGuiKey_H)) {
            if(app.selection_range_start != -1 && app.selection_range_end != -1) {
                for(int i = app.selection_range_start; i <= app.selection_range_end; ++i) {
                    app.scene_objects[i]->setDisabledVisibility(!app.scene_objects[i]->isVisible());
                }
            } else {
                SceneObject* selected_obj = app.getSelectedObject();
                if(selected_obj)
                    selected_obj->setDisabledVisibility(!selected_obj->isVisible());
            }
        }

        const ImVec2 label_size = ImGui::CalcTextSize("##empty", NULL, true);
        const float square_size = ImGui::GetFrameHeight();
        const ImVec2 checkBox_size(square_size + (label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f), label_size.y + style.FramePadding.y * 2.0f);
        const float width = ImGui::GetWindowContentRegionWidth();

        for(int32_t i = 0; i < app.scene_objects.size(); ++i)
        {
            ImGui::PushID(i);
            ImVec2 cursor = ImGui::GetCursorPos();

            SceneObject& scene_object = *app.scene_objects[i];
            bool is_selected = (i == app.selected_object_index || (app.selection_range_start <= i && i <= app.selection_range_end));
            if(ImGui::Selectable(scene_object.getName().c_str(), is_selected, 0, ImVec2(width - checkBox_size.x - style.FramePadding.x, checkBox_size.y))) {
                if(io.KeyShift) {
                    app.selection_range_start = app.selected_object_index;
                    app.selection_range_end = i;

                    if(app.selection_range_start > app.selection_range_end) {
                        std::swap(app.selection_range_start, app.selection_range_end);
                    }
                } else {
                    app.selection_range_start = -1;
                    app.selection_range_end = -1;
                }
                app.selected_object_index = i;
            }

            if(is_selected)
                ImGui::SetItemDefaultFocus();

            ImVec2 saved_cursor = ImGui::GetCursorPos();
            cursor[0] += (width - checkBox_size.x);
            ImGui::SetCursorPos(cursor);

            bool is_visible = scene_object.isVisible();
            if(ImGui::Checkbox("##empty", &is_visible)) {
                bool obj_visible = scene_object.isVisible();
                if(io.KeyCtrl && app.selection_range_start != -1 && app.selection_range_end != -1) {
                    for(int i = app.selection_range_start; i <= app.selection_range_end; ++i) {
                        app.scene_objects[i]->setVisibility(obj_visible ? SceneObject::HIDDEN : SceneObject::VISIBLE);
                    }
                } else {
                    scene_object.setVisibility(obj_visible ? SceneObject::HIDDEN : SceneObject::VISIBLE);
                }
            }
            ImGui::SetCursorPos(saved_cursor);
            ImGui::PopID();
        }

        ImGui::EndListBox();
    }

    ImGui::ResizeHandle("##resize_handle", &select_size);

    if(ImGui::Button("Add Files"))
    {
        Timer timer;
        std::vector<boost::filesystem::path> selected_files;
        nfdfilteritem_t filterItem[1] = {{"NRRD", "nrrd"}};
        boost::filesystem::path default_path = NrrdResource::getRootDirectory() / "defective_skull";
        ui_file_dialog_select_multiple(selected_files, default_path.string(), filterItem, 1);
        app.addNrrdObjects(selected_files);
        std::cout << "Loading all skull took " << timer.stop().toString() << std::endl;
    }

    bool object_selected = app.selected_object_index != -1;
    bool multiple_objects_selected = app.selection_range_start != -1 && app.selection_range_end != -1;
    if(object_selected && !multiple_objects_selected) {
        ImGui::BeginDisabled(app.getSelectedObject()->getType() == SceneObject::AREA_PICKER);
        ImGui::SameLine();
        if(ImGui::Button("Delete Object")) {
            app.deleteSceneObject(app.selected_object_index);
            ImGui::EndDisabled();
            ImGui::End();
            return;
        }
        ImGui::EndDisabled();
    }

    if(multiple_objects_selected) {
        ImGui::BeginDisabled(app.getSelectedObject()->getType() == SceneObject::AREA_PICKER);
        ImGui::SameLine();
        if(ImGui::Button("Delete Objects")) {
            for(int i = app.selection_range_start, j = 0; i <= app.selection_range_end; ++i, ++j) {
                app.deleteSceneObject(i - j);
            }
            ImGui::EndDisabled();
            ImGui::End();
            return;
        }
        ImGui::EndDisabled();
    }

    if(object_selected)
    {
        SceneObject& scene_object = *app.getSelectedObject();
        bool is_visible = scene_object.isVisible();

        char obj_name_buffer[200];
        std::strcpy(obj_name_buffer, scene_object.name_.c_str());
        if(ImGui::InputText("Object Name", obj_name_buffer, IM_ARRAYSIZE(obj_name_buffer)))
            scene_object.name_ = std::string(obj_name_buffer);

        ImGui::Separator();
        if(ImGui::Checkbox("Visible", &is_visible))
            scene_object.setVisibility(is_visible ? SceneObject::HIDDEN : SceneObject::VISIBLE);

        ImGui::Checkbox("Wireframe", &scene_object.is_wireframe_);

        if(scene_object.getType() == SceneObject::AREA_PICKER) {
            ImGui::DragFloat3("Translation", (float*)&scene_object.getTranslation(), 0.03f, -50.0f, 50.0f);
            ImGui::DragFloat("Scale", &((AreaPicker*)&scene_object)->getRadius(), 0.1f, 0.0f, 100.0f);
        } else {
            ImGui::DragFloat3("Translation", (float*)&scene_object.getTranslation(), 0.03f, -50.0f, 50.0f);
            ImGui::DragFloat3("Rotation", (float*)&scene_object.getRotation(), 0.1f, -360.0f, 360.0f);
            ImGui::DragFloat3("Scale", (float*)&scene_object.getScale(), 0.01f, 0.0f, 10.0f);
        }

        ImGui::ColorEdit4("Object Color", (float*)&scene_object.color_[0]);

        if(scene_object.getType() == SceneObject::SKULL)
            ImGui::Text("Score: %f", scene_object.getActiveTransformation().score);

        if(scene_object.getType() == SceneObject::SEQUENCE_LINE_OBJECT)
        {
            SequenceLineObject* sequence = (SequenceLineObject*)&scene_object;
            int new_index = sequence->getSequenceIndex();
            if(ImGui::DragInt("Show Sequence Step", &new_index, 1, 0, sequence->getSize()))
                sequence->setSequenceIndex(new_index);
        }

        ImGui::Separator();
        ImGui::Text("Transformations:");
        static float transform_listbox_size = 50.0f;
        if(ImGui::BeginListBox("Transformations", ImVec2(-FLT_MIN, transform_listbox_size)))
        {
            for(int32_t i = 0; i < scene_object.transformations_.size(); ++i)
            {
                SceneObject::Transformation& transformation = scene_object.transformations_[i];
                scene_object.transformation_is_overriden = false;

                ImGui::PushID(i);
                bool is_selected = (i == scene_object.selected_transformation_);
                if(ImGui::Selectable(transformation.name.c_str(), is_selected, 0))
                    scene_object.selected_transformation_ = i;

                if(is_selected)
                    ImGui::SetItemDefaultFocus();

                ImGui::PopID();
            }

            ImGui::EndListBox();
        }
        ImGui::ResizeHandle("##transformresize", &transform_listbox_size);

        bool transformation_button_disabled = scene_object.transformations_.size() <= 1;
        ImGui::BeginDisabled(transformation_button_disabled);
        {
            if(ImGui::Button("Remove Transformation"))
                scene_object.removeActiveTransformation();

                ImGui::EndDisabled();
        }

        char name_buffer[200];
        std::strcpy(name_buffer, scene_object.getActiveTransformation().name.c_str());
        if(ImGui::InputText("Name", name_buffer, IM_ARRAYSIZE(name_buffer)))
            scene_object.getActiveTransformation().name = std::string(name_buffer);
    }

    ImGui::End();
}