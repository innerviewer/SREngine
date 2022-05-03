//
// Created by Monika on 24.04.2022.
//

#include <Render/Implementations/VulkanRender.h>

void Framework::Graphics::Impl::VulkanRender::UpdateUBOs() {
    if (m_currentCamera) {
        m_currentCamera->UpdateShader<ProjViewUBO>(m_shaders[Shader::StandardID::DebugWireframe]);
        m_currentCamera->UpdateShader<SkyboxUBO>(m_shaders[Shader::StandardID::Skybox]);

        for (auto const& [shader, subCluster] : m_geometry.m_subClusters) {
            if (!shader || !shader->Ready()) {
                continue;
            }

            shader->SetMat4("VIEW_MATRIX", m_currentCamera->GetViewTranslate());
            shader->SetMat4("PROJECTION_MATRIX", m_currentCamera->GetProjection());
            shader->SetFloat("TIME", clock());

            for (auto const& [key, meshGroup] : subCluster.m_groups) {
                for (const auto &mesh : meshGroup) {
                    mesh->GetMaterial()->Use();

                    shader->SetMat4("MODEL_MATRIX", mesh->GetModelMatrix());

                    if (auto&& ubo = mesh->GetUBO()) {
                        m_env->BindUBO(ubo);
                    }

                    shader->Flush();
                }
            }
        }
    }
}

void Framework::Graphics::Impl::VulkanRender::DrawGeometry()  {
    static Environment* env = Environment::Get();

    for (auto const& [shader, subCluster] : m_geometry.m_subClusters) {
        if (!shader || shader && !shader->Use()) {
            continue;
        }

        for (auto const& [key, meshGroup] : subCluster.m_groups) {
            env->BindVBO((*meshGroup.begin())->GetVBO<true>());
            env->BindIBO((*meshGroup.begin())->GetIBO<true>());

            for (const auto &mesh : meshGroup)
                mesh->DrawVulkan();
        }

        env->UnUseShader();
    }
}
