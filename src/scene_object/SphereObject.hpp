#include "SceneObject.hpp"
#include "opengl_object/Sphere.hpp"

#ifndef SPHERE_OBJECT_H
#define SPHERE_OBJECT_H

class SphereObject : public SceneObject
{
    protected:
        Sphere sphere_;
        float radius_ = 1.0f;

    public:
        SphereObject(const std::string& name, Type type = Type::SPHERE_OBJECT) : SceneObject(name, &sphere_, type) {}

        Sphere* getSphere()
        {
            return &sphere_;
        }

        void setRadius(float radius)
        {
            radius_ = radius;
        }

        float& getRadius()
        {
            return radius_;
        }

        float getRadius() const
        {
            return radius_;
        }

        virtual void updateModelMatrix() override
        {
            glm::mat4 scale = glm::scale(glm::mat4(1.0), glm::vec3(radius_));
            glm::mat4 translation = glm::translate(glm::mat4(1.0), getTranslation());

            model_ = xyzToxzy(translation * scale);
        }
};
#endif // SPHERE_OBJECT_H