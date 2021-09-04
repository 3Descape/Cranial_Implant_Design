#include "SceneObject.hpp"
#include "opengl_object/Line.hpp"

#ifndef LINE_OBJECT_H
#define LINE_OBJECT_H

class LineObject : public SceneObject
{
    protected:
        Line line_;
        float width_ = 1.0f;

    public:
        LineObject(
            const std::string& name,
            const OpenglObjectDescriptor& descriptor,
            const std::vector<UniformLayout>& uniform_layout = UniformLayoutDefault,
            Type type = Type::LINE_OBJECT
            ) :
            SceneObject(name, &line_, type),
            line_(descriptor, uniform_layout) {}

        Line* getLine()
        {
            return &line_;
        }

        void setWidth(float width)
        {
            width_ = width;
        }

        float& getWidth()
        {
            return width_;
        }

        float getWidth() const
        {
            return width_;
        }
};
#endif // LINE_OBJECT_H