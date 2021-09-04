#include "util/util_primitives.hpp"

#ifndef SPHERE_H
#define SPHERE_H

class Sphere : public OpenGLObject
{
    private:
        inline static std::vector<float> data_;
        inline static std::vector<glm::uvec3> indices_;

    public:
        Sphere(const std::vector<UniformLayout>& uniform_layout = UniformLayoutDefault) : OpenGLObject(uniform_layout)
        {
            generateBuffers();
            if(data_.empty() || indices_.empty())
                generateSphere(40, 40, data_, indices_);

            OpenglObjectDescriptor descriptor = {
                ArrayLayout::AoS,
                data_.data(),
                data_.size() * sizeof(data_[0]),
                indices_.data(),
                indices_.size() * sizeof(indices_[0]),
                indices_.size() * 3,
                {
                    { "position", AttributeLayout::POSITION, 3, GL_FLOAT, sizeof(float), 0 },
                    { "normal", AttributeLayout::NORMAL, 3, GL_FLOAT, sizeof(float), 0 },
                },
            };

            element_count_ = descriptor.indices_count;
            bufferData(descriptor);
        }

        Sphere(const Sphere& other) = default;
        Sphere& operator=(const Sphere& other) = default;

        void draw() const override
        {
            drawIndexed();
        }
};

#endif // SPHERE_H