//
// Created by Monika on 12.04.2024.
//

#include <Core/States/SubmitState.h>

#include <Graphics/Window/Window.h>
#include <Graphics/Render/RenderScene.h>
#include <Graphics/Pipeline/Pipeline.h>

#include <Core/Engine.h>
#include <Core/World/EngineScene.h>

namespace SR_CORE_NS {
    SR_UTILS_NS::ThreadWorkerResult SubmitState::ExecuteImpl() {
        auto&& pEngine = GetContext().GetPointer<Engine>();

        auto&& pWindow = pEngine->GetMainWindow();
        if (!pWindow || !pWindow->IsVisible()) {
            return SR_UTILS_NS::ThreadWorkerResult::Break;
        }

        auto&& pRenderScene = pEngine->GetRenderScene();
        if (!pRenderScene) {
            return SR_UTILS_NS::ThreadWorkerResult::Break;
        }

        pRenderScene->Submit();

        SR_TRACY_PLOT("Draw calls", static_cast<int64_t>(pRenderScene->GetPipeline()->GetBuildState().drawCalls));

        return SR_UTILS_NS::ThreadWorkerResult::Success;
    }
}
