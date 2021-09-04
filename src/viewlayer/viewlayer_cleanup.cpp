#include <iostream>
#include <algorithm>
#include <numeric>
#include <vector>

#include "application.hpp"
#include "scene_object/Skull.hpp"

void viewlayer_draw_cleanup()
{
    ImGui::Begin("Cleanup", NULL);

    std::vector<float> scores;
    for(size_t index = 0; index < app.scene_objects.size(); ++index)
    {
        if(app.scene_objects[index]->getType() != SceneObject::SKULL || index == app.target_index) continue;

        scores.push_back(app.scene_objects[index]->getActiveTransformation().score);
    }

    static int percentile_threshold = 60;
    ImGui::SliderInt("Nth. Percentile", &percentile_threshold, 0, 100);

    float mean = 0.0f;
    float percentile = -1.0f;
    int percentile_index = -1;
    int start_index = 0;
    int valid_count = 0;

    if(scores.size())
    {
        std::sort(scores.begin(), scores.end());
        while(start_index < scores.size() && scores[start_index] == 0) {
            start_index++;
        }
        // std::cout << "Start index: " << start_index << std::endl;
        valid_count = scores.size() - start_index;
        mean = std::accumulate(scores.begin() + start_index, scores.end(), 0.0f) / valid_count;
        percentile_index = start_index + floor((valid_count - 1) * (percentile_threshold / 100.0f));

        if(percentile_index >= 0)
            percentile = scores[percentile_index];
    }

    ImGui::Text("Mean: %0.2f", mean);
    ImGui::Text("Error: %0.2f, at index: %d", percentile, percentile_index);
    ImGui::Text("Valid skulls left after deletion: %d/%d", percentile_index + 1, scores.size());

    if(ImGui::Button("Delete above nth. percentile"))
    {
        for(size_t index = 0; index < app.scene_objects.size(); ++index)
        {
            if((app.scene_objects[index]->getType() != SceneObject::SKULL) || (app.getTargetObject() == app.scene_objects[index])) continue;

            if(app.scene_objects[index]->getActiveTransformation().score > percentile || app.scene_objects[index]->getActiveTransformation().score == 0) {
                app.deleteSceneObject(index);
                index--;
            }
        }
    }

    ImGui::End();
}