#include "OpenGLObject.hpp"

#ifndef LINE_H
#define LINE_H
class Line : public OpenGLObject
{
    public:
        Line() {}
        Line(const OpenglObjectDescriptor& descriptor, const std::vector<UniformLayout>& uniform_layout = UniformLayoutDefault) : OpenGLObject(uniform_layout, descriptor.shader)
        {
            generateBuffers(descriptor.buffer_flag);
            element_count_ = descriptor.indices_count;
            bufferData(descriptor);
        };

        Line(const Line& other) = default;
        Line& operator=(const Line& other) = default;

        void draw() const override
        {
            drawLine(3);
        }
};

#endif // LINE_H