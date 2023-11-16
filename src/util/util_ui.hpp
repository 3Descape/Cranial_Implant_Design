
#include <vector>
#include <boost/filesystem.hpp>

#include <nfd.h>
#include "imgui/imgui.h"
#include "logger/logger.hpp"


#ifndef UTIL_UI
#define UTIL_UI

inline int ui_file_dialog_single_select(boost::filesystem::path& output_path, const std::string& default_path, nfdfilteritem_t* filterItem = nullptr, nfdfiltersize_t filterCount = 0)
{
    nfdchar_t* path = NULL;
    nfdresult_t result = NFD_OpenDialog(&path, filterItem, filterCount, default_path.empty() ? nullptr : default_path.c_str());

    if (result == NFD_CANCEL)
    {
        LOG_INFO("NFD: User pressed cancel.");
        return NFD_CANCEL;
    }
    else if(result == NFD_ERROR)
    {
        LOG_ERROR("NFD: {}", NFD_GetError());
        return NFD_ERROR;
    }

    output_path = boost::filesystem::path(path);
    NFD_FreePath(path);

    return NFD_OKAY;
}

inline int ui_file_dialog_select_multiple(std::vector<boost::filesystem::path>& output_paths, const std::string& default_path, nfdfilteritem_t* filterItem = nullptr, nfdfiltersize_t filterCount = 0)
{
    const nfdpathset_t* outPaths;
    nfdresult_t result = NFD_OpenDialogMultiple(&outPaths, filterItem, filterCount, default_path.empty() ? nullptr : default_path.c_str());

    if (result == NFD_CANCEL) {
        return NFD_CANCEL;
    }
    else if(result == NFD_ERROR) {
        LOG_ERROR("NFD: {}", NFD_GetError());
        return NFD_ERROR;
    }

    unsigned i = 0;
    nfdchar_t* path;
    nfdpathsetenum_t enumerator;
    NFD_PathSet_GetEnum(outPaths, &enumerator);
    while (NFD_PathSet_EnumNext(&enumerator, &path) && path) {
        output_paths.push_back(boost::filesystem::path(path));
        NFD_PathSet_FreePath(path);
    }

    NFD_PathSet_FreeEnum(&enumerator);
    NFD_PathSet_Free(outPaths);

    return NFD_OKAY;
}

inline int ui_file_dialog_save(boost::filesystem::path& output_path, const std::string& default_path, const std::string& default_name, nfdfilteritem_t* filterItem = nullptr, nfdfiltersize_t filterCount = 0)
{
    nfdchar_t* path = NULL;
    nfdresult_t result = NFD_SaveDialog(&path, filterItem, filterCount, default_path.empty() ? nullptr : default_path.c_str(), default_name.empty() ? nullptr : default_name.c_str());

    if (result == NFD_CANCEL) {
        return NFD_CANCEL;
    }
    else if(result == NFD_ERROR) {
        LOG_ERROR("NFD: {}", NFD_GetError());
        return NFD_ERROR;
    }

    output_path = boost::filesystem::path(path);
    NFD_FreePath(path);

    return NFD_OKAY;
}

inline int ui_file_dialog_folder_select(boost::filesystem::path& output_path, const std::string& default_path)
{
    nfdchar_t* path = NULL;
    nfdresult_t result = NFD_PickFolder(&path, default_path.empty() ? nullptr : default_path.c_str());

    if (result == NFD_CANCEL) {
        return NFD_CANCEL;
    }
    else if(result == NFD_ERROR)
    {
        LOG_ERROR("NFD: {}", NFD_GetError());
        return NFD_ERROR;
    }

    output_path = boost::filesystem::path(path);
    NFD_FreePath(path);

    return NFD_OKAY;
}

namespace ImGui {
    inline bool ResizeHandle(const char* label, float* size, ImVec2 handle_size = ImVec2(-FLT_MIN, 3)) {
        ImGui::PushID(label);
        ImGuiID id = ImGui::GetID("_state_is_resizing");
        ImGuiStorage* storage = ImGui::GetStateStorage();
        bool is_resizing = storage->GetBool(id, false);
        if(is_resizing)
            ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ResizeGripHovered));
        else
            ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_Separator));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetStyleColorVec4(ImGuiCol_SeparatorHovered));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGui::GetStyleColorVec4(ImGuiCol_SeparatorActive));

        ImGui::Button("##_resize_handle", handle_size); // TODO: make it, such that is has no tapindex?
        if(ImGui::IsItemHovered() || is_resizing) {
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);

            if(ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                ImGui::ResetMouseDragDelta(ImGuiMouseButton_Left);
                storage->SetBool(id, true);
            }

            if(ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
                storage->SetBool(id, false);
            }

            if(is_resizing) {
                *size = std::max(5.0f , *size + ImGui::GetMouseDragDelta(ImGuiMouseButton_Left).y);
                ImGui::ResetMouseDragDelta(ImGuiMouseButton_Left);
            }
        }
        ImGui::PopStyleColor(3);
        ImGui::PopID();

        return is_resizing;
    }
}

#endif // UTIL_UI