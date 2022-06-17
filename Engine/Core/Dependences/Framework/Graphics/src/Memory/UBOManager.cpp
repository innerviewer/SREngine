//
// Created by Monika on 10.06.2022.
//

#include <Memory/UBOManager.h>
#include <Render/Camera.h>
#include <Environment/Environment.h>

namespace SR_GRAPH_NS::Memory {
    UBOManager::UBOManager() {
        m_virtualTable.max_load_factor(0.9f);
        m_virtualTable.reserve(5000);
    }

    void UBOManager::SetCurrentCamera(Camera *pCamera) {
        m_camera = pCamera;
    }

    UBOManager::VirtualUBO UBOManager::AllocateUBO(uint32_t uboSize, uint32_t samples) {
        if (!m_camera) {
            SR_ERROR("UBOManager::AllocateUBO() : camera is nullptr!");
            return SR_ID_INVALID;
        }

        const VirtualUBO virtualUbo = GenerateUnique();

        Descriptor descriptor = SR_ID_INVALID;
        UBO ubo = SR_ID_INVALID;

        if (!AllocMemory(&ubo, &descriptor, uboSize, samples)) {
            SR_ERROR("UBOManager::AllocateUBO() : failed to allocate memory!");
            return SR_ID_INVALID;
        }

        VirtualUBOInfo virtualUboInfo;

        virtualUboInfo.m_samples = samples;
        virtualUboInfo.m_uboSize = uboSize;
        virtualUboInfo.m_data.insert(std::make_pair(
                m_camera,
                std::make_pair(ubo, descriptor))
        );

        m_virtualTable.insert(std::make_pair(
                virtualUbo,
                std::move(virtualUboInfo))
        );

        return virtualUbo;
    }

    bool UBOManager::FreeUBO(UBOManager::VirtualUBO *virtualUbo) {
        if (!SRVerifyFalse(!virtualUbo)) {
            return false;
        }

        auto&& pIt = m_virtualTable.find(*virtualUbo);
        if (pIt == std::end(m_virtualTable)) {
            SRHalt("UBOManager::FreeUBO() : ubo not found!");
            return false;
        }

        auto&& env = Environment::Get();

        auto&& info = pIt->second;
        for (auto&& [pCamera, data] : info.m_data) {
            auto&& [ubo, descriptor] = data;
            FreeMemory(&ubo, &descriptor);
        }

        m_virtualTable.erase(pIt);

        *virtualUbo = SR_ID_INVALID;

        return true;
    }

    UBOManager::VirtualUBO UBOManager::GenerateUnique() const {
        volatile VirtualUBO virtualUbo = SR_ID_INVALID;

        auto&& random = SR_UTILS_NS::Random::Instance();

        while (virtualUbo == SR_ID_INVALID) {
            VirtualUBO unique = random.Int32();

            /// можно использовать только положительные индексы
            if (unique < 0) {
                unique = -unique;
            }

            SRAssertOnce(unique >= 0);

            if (m_virtualTable.count(unique) == 0 && unique != SR_ID_INVALID) {
                virtualUbo = unique;
                break;
            }

            SR_WARN("UBOManager::GenerateUnique() : collision detected!");
        }

        return virtualUbo;
    }

    bool UBOManager::AllocMemory(UBO *ubo, Descriptor *descriptor, uint32_t uboSize, uint32_t samples) {
        auto&& env = Environment::Get();

        if (uboSize > 0) {
            if (*descriptor = env->AllocDescriptorSet({DescriptorType::Uniform}); *descriptor < 0) {
                SR_ERROR("UBOManager::AllocMemory() : failed to allocate descriptor set!");
                return false;
            }

            if (*ubo = env->AllocateUBO(uboSize); *ubo < 0) {
                SR_ERROR("UBOManager::AllocMemory() : failed to allocate uniform buffer object!");
                return false;
            }
        }
        else if (samples > 0) {
            if (*descriptor = env->AllocDescriptorSet({DescriptorType::CombinedImage}); *descriptor < 0) {
                SR_ERROR("UBOManager::AllocMemory() : failed to allocate descriptor set!");
                return false;
            }
        }

        return true;
    }

    void UBOManager::BindUBO(VirtualUBO virtualUbo) {
        if (!m_camera) {
            SR_ERROR("UBOManager::AllocateUBO() : camera is nullptr!");
            return;
        }

        auto&& pIt = m_virtualTable.find(virtualUbo);
        if (pIt == std::end(m_virtualTable)) {
            SRHalt("UBOManager::BindUBO() : ubo not found!");
            return;
        }

        VirtualUBOInfo& info = pIt->second;
    retry:
        auto&& cameraIt = info.m_data.find(m_camera);
        if (cameraIt == std::end(info.m_data))
        {
            Descriptor descriptor = SR_ID_INVALID;
            UBO ubo = SR_ID_INVALID;

            if (!AllocMemory(&ubo, &descriptor, info.m_uboSize, info.m_samples)) {
                SR_ERROR("UBOManager::BindUBO() : failed to allocate memory!");
                return;
            }

            info.m_data.insert(std::make_pair(
                    m_camera,
                    std::make_pair(ubo, descriptor))
            );

            goto retry;
        }

        auto&& env = Environment::Get();

        auto&& [ubo, descriptor] = cameraIt->second;

        if (ubo != SR_ID_INVALID) {
            env->BindUBO(ubo);
        }

        if (descriptor != SR_ID_INVALID) {
            env->BindDescriptorSet(descriptor);
        }
    }

    UBOManager::VirtualUBO UBOManager::ReAllocateUBO(VirtualUBO virtualUbo, uint32_t uboSize, uint32_t samples) {
        if (virtualUbo == SR_ID_INVALID) {
            return AllocateUBO(uboSize, samples);
        }

        auto&& pIt = m_virtualTable.find(virtualUbo);
        if (pIt == std::end(m_virtualTable)) {
            SRHalt("UBOManager::ReAllocateUBO() : ubo not found!");
            return virtualUbo;
        }

        auto&& info = pIt->second;
        for (auto&& [pCamera, data] : info.m_data) {
            auto&& [ubo, descriptor] = data;
            FreeMemory(&ubo, &descriptor);

            if (!AllocMemory(&ubo, &descriptor, info.m_uboSize, info.m_samples)) {
                SR_ERROR("UBOManager::ReAllocateUBO() : failed to allocate memory!");
            }
        }

        return virtualUbo;
    }

    void UBOManager::FreeMemory(UBOManager::UBO *ubo, UBOManager::Descriptor *descriptor) {
        auto&& env = Environment::Get();

        if (*ubo != SR_ID_INVALID && !env->FreeUBO(ubo)) {
            SR_ERROR("UBOManager::FreeMemory() : failed to free ubo!");
        }

        if (*descriptor != SR_ID_INVALID && !env->FreeDescriptorSet(descriptor)) {
            SR_ERROR("UBOManager::FreeMemory() : failed to free descriptor!");
        }
    }
}