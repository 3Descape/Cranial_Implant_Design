#include "MeshObject.hpp"

#ifndef MESH_OBJECT_WITH_DATA_H
#define MESH_OBJECT_WITH_DATA_H

class MeshObjectWithData : public MeshObject
{
    protected:
        std::vector<glm::vec3> points_;
        std::vector<glm::uvec3> triangles_;

    public:
        MeshObjectWithData(
            const std::string& name,
            const OpenglObjectDescriptor& descriptor,
            const std::vector<UniformLayout>& uniform_layout = UniformLayoutDefault)
            : MeshObject(name, descriptor, uniform_layout, Type::MESH_OBJECT_WITH_DATA) {}

        void setData(const std::vector<glm::vec3>& points, const std::vector<glm::uvec3>& triangles)
        {
            points_ = points;
            triangles_ = triangles;
        }

        const std::vector<glm::vec3>& getPoints() const {
            return points_;
        }

        const std::vector<glm::uvec3>& getTriangles() const {
            return triangles_;
        }
};
#endif // MESH_OBJECT_WITH_DATA_H