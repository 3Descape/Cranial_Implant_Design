#include "Ray.hpp"
#include "MeshObject.hpp"

#ifndef SKULL_H
#define SKULL_H

class Skull : public MeshObject
{
    private:
        std::vector<glm::vec3> points_;
        std::vector<glm::uvec3> triangles_;
        std::vector<bool> mask_;
        size_t mask_active_points_ = 0;
        SoA_Region_List regions_;

    public:
        Skull(const std::string& name, const OpenglObjectDescriptor& descriptor) :
            MeshObject(
                name,
                descriptor,
                {},
                Type::SKULL
            ),
            regions_(descriptor.regions)
            {}

        virtual void draw(const glm::mat4& view, const glm::mat4& projection) override;
        IntersectionPoint intersect(const Ray& ray);
        void setMeshData(const std::vector<glm::vec3>& points, const std::vector<glm::uvec3>& triangles);
        int updateColorBuffer();
        void updateSelectionMaskWithUserSelection();
        void updateSelectionMaskWithAreaPicker();
        const std::vector<bool>& getMaskReference() const;
        size_t getMaskActiveCount() const;
        const std::vector<glm::vec3>& getPoints() const;
        const std::vector<glm::uvec3>& getTriangles() const;
        bool isTarget() const;
};
#endif