#include "SceneObject.hpp"
#include "opengl_object/Mesh.hpp"

#include "SoA.hpp"

#ifndef MESH_OBJECT_H
#define MESH_OBJECT_H

class MeshObject : public SceneObject
{
    protected:
        Mesh mesh_;

    public:
        MeshObject(
            const std::string& name,
            const OpenglObjectDescriptor& descriptor,
            const std::vector<UniformLayout>& uniform_layout = UniformLayoutDefault,
            Type type = Type::MESH_OBJECT)
            :
            SceneObject(name, &mesh_, type),
            mesh_(descriptor, uniform_layout) {}

        Mesh* getMesh()
        {
            return &mesh_;
        }
};
#endif // MESH_OBJECT_H