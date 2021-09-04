#include "SceneObject.hpp"
#include "opengl_object/Point.hpp"

#ifndef POINT_OBJECT_H
#define POINT_OBJECT_H

class PointObject : public SceneObject
{
    protected:
        Point point_;
        float size_ = 1.0f;

    public:
        PointObject(
            const std::string& name,
            const OpenglObjectDescriptor& descriptor,
            const std::vector<UniformLayout>& uniform_layout = UniformLayoutDefault,
            Type type = Type::POINT_OBJECT
        ) :
            SceneObject(name, &point_, type),
            point_(descriptor, uniform_layout) {}

        Point* getPoint()
        {
            return &point_;
        }

        void setSize(float size)
        {
            size_ = size;
        }

        float& getSize()
        {
            return size_;
        }

        float getSize() const
        {
            return size_;
        }
};
#endif // POINT_OBJECT_H