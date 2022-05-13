//
// Created by Monika on 24.03.2022.
//

#ifndef SRENGINE_RAWMESH_H
#define SRENGINE_RAWMESH_H

#include <ResourceManager/IResource.h>
#include <Utils/Vertices.hpp>
#include <Types/SafePointer.h>

namespace Assimp {
    class Importer;
}

class aiScene;

namespace SR_WORLD_NS {
    class Scene;
}

namespace SR_UTILS_NS::Types {
    class RawMesh : public IResource {
        using CallbackFn = std::function<bool(const aiScene*)>;
        using ScenePtr = SR_HTYPES_NS::SafePtr<SR_WORLD_NS::Scene>;

    private:
        RawMesh();
        ~RawMesh() override;

    public:
        static RawMesh *Load(const std::string &path);

    public:
        bool Access(const CallbackFn& fn) const;
        uint32_t GetMeshesCount() const;
        std::string GetGeometryName(uint32_t id) const;

        std::vector<SR_UTILS_NS::Vertex> GetVertices(uint32_t id) const;
        std::vector<uint32_t> GetIndices(uint32_t id) const;

        SR_NODISCARD uint32_t GetVerticesCount(uint32_t id) const;
        SR_NODISCARD uint32_t GetIndicesCount(uint32_t id) const;

        SR_NODISCARD float_t GetScaleFactor() const;

    protected:
        bool Unload() override;
        bool Load() override;

    private:
        const aiScene* m_scene;
        Assimp::Importer* m_importer;

    };
}

#endif //SRENGINE_RAWMESH_H
