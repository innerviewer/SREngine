//
// Created by Monika on 16.11.2021.
//

#ifndef SR_ENGINE_VISUALCHUNK_H
#define SR_ENGINE_VISUALCHUNK_H

#include <Utils/World/Chunk.h>
#include <Utils/Math/Vector3.h>

namespace SR_GTYPES_NS {
    class DebugWireframeMesh;
}

namespace SR_GRAPH_NS {
    class RenderScene;
}

namespace SR_WORLD_NS {
    class Region;
}

namespace SR_CORE_NS {
    class VisualChunk : public SR_WORLD_NS::Chunk {
        using RenderScenePtr = SR_HTYPES_NS::SafePtr<SR_GRAPH_NS::RenderScene>;
    public:
        explicit VisualChunk(SRChunkAllocArgs);
        ~VisualChunk() override;

    private:
        SR_GTYPES_NS::DebugWireframeMesh* m_loadMesh = nullptr;
        SR_GTYPES_NS::DebugWireframeMesh* m_stayMesh = nullptr;

    private:
        void SetFacesVisible(bool value);
        void SetLoadVisible(bool value);
        void UpdateFacesPos();
        void UpdateLoadPos();

        bool ApplyOffset() override;
        void Reload() override;
        void OnExit() override;
        void OnEnter() override;
        bool Unload() override;

        void Update(float_t dt) override;

    };
}

#endif //SR_ENGINE_VISUALCHUNK_H
