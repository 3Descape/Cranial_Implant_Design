#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

#include "Shader.hpp"
#include "OpenGLObject.hpp"

#ifndef POINT_H
#define POINT_H
class Point : public OpenGLObject
{
    public:
        Point() {}
        Point(const OpenglObjectDescriptor& descriptor, const std::vector<UniformLayout>& uniform_layout = UniformLayoutDefault) : OpenGLObject(uniform_layout, descriptor.shader)
        {
            generateBuffers(descriptor.buffer_flag);
            element_count_ = descriptor.indices_count;
            bufferData(descriptor);
        };

        Point(const Point& other) = default;
        Point& operator=(const Point& other) = default;

        void draw() const override
        {
            drawPoint(6);
        }
};

#endif // POINT_H