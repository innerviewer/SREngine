//
// Created by Monika on 06.05.2022.
//

#include <Types/Framebuffer.h>
#include <Environment/Environment.h>
#include <Render/Shader.h>

namespace SR_GTYPES_NS {
    Framebuffer::~Framebuffer() {
        SRAssert(m_frameBuffer == SR_ID_INVALID);

        for (auto&& color : m_colors) {
            SRAssert(color == SR_ID_INVALID);
        }
    }

    Framebuffer *Framebuffer::Create(uint32_t images, const Math::IVector2 &size) {
        return Create(images, size, std::string());
    }

    Framebuffer::Ptr Framebuffer::Create(uint32_t images, const Math::IVector2 &size, const std::string& shaderPath) {
        Framebuffer* fbo = new Framebuffer();

        SRAssert(!size.HasZero() && !size.HasNegative() && images > 0);

        fbo->SetSize(size);
        fbo->SetImagesCount(images);

        if (!shaderPath.empty() && (fbo->m_shader = Shader::Load(shaderPath))) {
            fbo->m_shader->AddUsePoint();
        }

        return fbo;
    }

    bool Framebuffer::Bind() {
        if (m_hasErrors) {
            return false;
        }

        if ((!m_isInit || m_needResize) && !Init()) {
            SR_ERROR("Framebuffer::Bind() : failed to initialize framebuffer!");
        }

        Environment::Get()->BindFrameBuffer(m_frameBuffer);

        return true;
    }

    bool Framebuffer::Init() {
        if (!OnResize()) {
            SR_ERROR("Framebuffer::OnResize() : failed to resize frame buffer!");
            return false;
        }

        return true;
    }

    void Framebuffer::Free() {
        if (!m_isInit) {
            return;
        }

        auto&& environment = Environment::Get();

        if (m_frameBuffer != SR_ID_INVALID) {
            SRVerifyFalse(environment->FreeFBO(m_frameBuffer));
            m_frameBuffer = SR_ID_INVALID;
        }

        for (auto&& color : m_colors) {
            if (color == SR_ID_INVALID) {
                continue;
            }

            SRVerifyFalse(environment->FreeTexture(color));

            color = SR_ID_INVALID;
        }

        if (m_shader) {
            m_shader->RemoveUsePoint();
            m_shader = nullptr;
        }

        delete this;
    }

    bool Framebuffer::OnResize() {
        if (m_colors.empty()) {
            SR_ERROR("Framebuffer::OnResize() : colors count isn't set!");
            m_hasErrors = true;
            return false;
        }

        if (m_size.HasZero() || m_size.HasNegative()) {
            SR_ERROR("Framebuffer::OnResize() : incorrect FBO size!");
            m_hasErrors = true;
            return false;
        }

        if (!Environment::Get()->CreateFrameBuffer(m_size.ToGLM(), m_depth, m_frameBuffer, m_colors)) {
            SR_ERROR("Framebuffer::OnResize() : failed to create frame buffer!");
            m_hasErrors = true;
            return false;
        }

        m_hasErrors = false;
        m_needResize = false;

        return true;
    }

    void Framebuffer::SetSize(const Math::IVector2 &size) {
        m_size = size;
        m_needResize = true;
    }

    void Framebuffer::SetImagesCount(uint32_t count) {
        if (m_isInit) {
            SRVerifyFalse2(false, "Frame buffer are initialized!");
            return;
        }

        m_colors.resize(count);
    }

    bool Framebuffer::BeginRender() {
        auto&& env = Environment::Get();

        env->BeginRender();

        env->SetViewport(m_size.x, m_size.y);
        env->SetScissor(m_size.x, m_size.y);

        return true;
    }

    void Framebuffer::EndRender() {
        Environment::Get()->EndRender();
    }

    void Framebuffer::Draw() {
        if (m_hasErrors) {
            return;
        }

        if (!m_shader) {
            if (!(m_shader = Shader::Load("Engine/framebuffer.srsl"))) {
                m_hasErrors = false;
                return;
            }

            m_shader->AddUsePoint();
        }

        auto&& env = Environment::Get();

        if (!m_shader->Use()) {
            m_hasErrors = false;
            return;
        }

        env->ResetDescriptorSet();
        env->Draw(3);

        m_shader->UnUse();
    }
}