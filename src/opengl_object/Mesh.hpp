#include "OpenGLObject.hpp"

#ifndef MESH_H
#define MESH_H

class Mesh : public OpenGLObject
{
    public:
        Mesh(const OpenglObjectDescriptor& descriptor, const std::vector<UniformLayout>& uniform_layout = UniformLayoutDefault) : OpenGLObject(uniform_layout, descriptor.shader)
        {
            generateBuffers();
            element_count_ = descriptor.indices_count;
            bufferData(descriptor);
        }

        Mesh(const Mesh& other) = default;
        Mesh& operator=(const Mesh& other) = default;

        void draw() const override
        {
            drawIndexed();
        }
};

#endif // MESH_H