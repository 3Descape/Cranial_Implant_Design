#include "SphereObject.hpp"

#ifndef AREA_PICKER_H
#define AREA_PICKER_H

class AreaPicker : public SphereObject
{
    public:
        AreaPicker(const std::string& name) : SphereObject(name, AREA_PICKER)
        {
            radius_ = 13.6f;
            color_.a = 0.3f;
            sphere_.setShader(Shader::get("3D_position_normal"));
            sphere_.addUniformLayout({"color", UniformType::VEC4, &color_});

            transformations_.push_back( { glm::vec3(-2.5f, -11.49f, 22.23f), glm::vec3(0.0f), glm::vec3(1.0), "Test Transform" } );
        }
};
#endif