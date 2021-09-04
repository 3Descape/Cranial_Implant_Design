#include <string>
#include <glm/gtx/euler_angles.hpp>

#include "opengl_object/OpenGLObject.hpp"
#include "opengl_object/Mesh.hpp"
#include "../util/util_opengl.hpp"
#include "Shader.hpp"

#ifndef SCENE_OBJECT_H
#define SCENE_OBJECT_H

class SceneObject
{
    public:
        typedef enum Type
        {
            UNSPECIFIED,
            SPHERE_OBJECT,
            MESH_OBJECT,
            MESH_OBJECT_WITH_DATA,
            LINE_OBJECT,
            SEQUENCE_LINE_OBJECT,
            POINT_OBJECT,
            SKULL,
            AREA_PICKER,
        } Type;

        typedef struct Transformation
        {
            glm::vec3 translation;
            glm::vec3 rotation;
            glm::vec3 scale;
            std::string name;
            float score;
        } Transformation;

        typedef enum Visibility
        {
            VISIBLE,
            HIDDEN,
            DISABLED,
        } Visibility;

    private:
        const Type type_;

    public:
        OpenGLObject* object_ = nullptr;
        Visibility visibility_ = VISIBLE;
        bool is_wireframe_ = false;
        std::string name_;
        int selected_transformation_ = 0;
        bool transformation_is_overriden = false;
        Transformation transformation_override;
        std::vector<Transformation> transformations_ = { {glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(1.0f), "Default"} };
        glm::mat4 model_ = glm::mat4(1.0f);
        glm::vec4 color_ = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

        SceneObject(const Type& type) : type_(type) {}
        SceneObject(const std::string& name, OpenGLObject* object, const Type& type = UNSPECIFIED) : object_(object), name_(name), type_(type) {}
        SceneObject(const SceneObject& obj) = default;
        ~SceneObject() = default;

        std::string getName() const { return name_; }
        Type getType() { return type_; }

        virtual void draw(const glm::mat4& view, const glm::mat4& projection)
        {
            updateModelMatrix();
            Shader* shader = object_->getShader();
            shader->use();
            object_->updateUniforms({
                {"model", { (void*)&model_, 1 }},
                {"view", { (void*)&view, 1 }},
                {"projection", { (void*)&projection, 1}}
            });

            if(is_wireframe_) OPENGL_CHECK(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
            object_->draw();
            if(is_wireframe_) OPENGL_CHECK(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
        }

        virtual void updateModelMatrix()
        {
            const Transformation& t = getActiveTransformation();
            glm::mat4 scale = glm::scale(glm::mat4(1.0), t.scale);
            glm::vec3 rotation_radians = glm::radians(t.rotation);
            glm::mat4 rotation = glm::eulerAngleYXZ(rotation_radians.y, rotation_radians.x, rotation_radians.z);
            glm::mat4 translation = glm::translate(glm::mat4(1.0), t.translation);

            model_ = xyzToxzy(translation * rotation * scale);
        }

        Transformation& getActiveTransformation()
        {
            if(transformation_is_overriden)
                return transformation_override;

            return transformations_[selected_transformation_];
        }

        void removeActiveTransformation()
        {
            if(transformations_.size() <= 1) return;
            transformations_.erase(transformations_.begin() + selected_transformation_);
            if(selected_transformation_ >= transformations_.size())
                selected_transformation_ = transformations_.size() - 1;
        }

        const Transformation& getActiveTransformation() const
        {
          return transformations_[selected_transformation_];
        }

        glm::vec3& getTranslation()
        {
            return getActiveTransformation().translation;
        }

        const glm::vec3& getTranslation() const
        {
          return getActiveTransformation().translation;
        }

        glm::vec3& getRotation()
        {
            return getActiveTransformation().rotation;
        }

        virtual glm::vec3& getScale()
        {
            return getActiveTransformation().scale;
        }

        bool isVisible() const
        {
            return visibility_ == SceneObject::VISIBLE;
        }

        void setDisabledVisibility(bool visible)
        {
            if(visible && visibility_ == SceneObject::DISABLED)
                visibility_ = SceneObject::VISIBLE;
            else if(!visible && visibility_ == SceneObject::VISIBLE)
                visibility_ = SceneObject::DISABLED;
        }

        void setVisibility(Visibility visibility)
        {
            visibility_ = visibility;
        }
};
#endif