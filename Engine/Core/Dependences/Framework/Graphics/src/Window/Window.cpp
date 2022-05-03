//
// Created by Nikita on 18.11.2020.
//

#include <Window/Window.h>

Framework::Graphics::Window::Window(
        std::string name,
        std::string icoPath,
        const Math::IVector2 &size,
        Framework::Graphics::Render *render,
        bool vsync, bool fullScreen, bool resizable,
        bool headerEnabled, uint8_t smoothSamples
    ) : m_env(Environment::Get())
    , m_size(size)
    , m_winName(std::move(name))
    , m_icoPath(std::move(icoPath))
    , m_render(render)
    , m_fullScreen(fullScreen)
    , m_vsync(vsync)
    , m_smoothSamples(smoothSamples)
    , m_resizable(resizable)
    , m_headerEnabled(headerEnabled)
{ }


bool Framework::Graphics::Window::Create() {
    if (m_isCreate) {
        SR_ERROR("Window::Create() : window are already create!");
        return false;
    }

    SR_INFO("Window::Create() : creating window...");

    if (!m_render->Create(this)) {
        SR_ERROR("Window::Create() : failed to create render!");
        m_hasErrors = true;
        return false;
    }

    Framework::Graphics::Environment::SetWinCallBack([this](Environment::WinEvents event, void* win, void* arg1, void* arg2){
        switch (event) {
            case Environment::WinEvents::Close:
                Debug::System("Window event: close window...");
                break;
            case Environment::WinEvents::Move: {
                int size[2] = {*(int *) arg1, *(int *) arg2};
                m_windowPos = { (int32_t)size[0], (int32_t)size[1] };
                m_env->SetWindowPosition((int)m_windowPos.x, (int)m_windowPos.y);
                break;
            }
            case Environment::WinEvents::Resize: {
                std::pair<int, int> size = {*(int *) arg1, *(int *) arg2};
                if (size.first > 0 && size.second > 0) {
                    for (auto camera : m_cameras.GetElements()) {
                        if (camera->IsAllowUpdateProjection()) {
                            camera->UpdateProjection(size.first, size.second);
                        }
                        camera->CompleteResize();
                    }
                }
                break;
            }
            case Environment::WinEvents::Scroll:
                break;
            case Environment::WinEvents::LeftClick:
                break;
            case Environment::WinEvents::RightClick:
                break;
            case Environment::WinEvents::Focus:
                this->m_isWindowFocus = *(bool*)(arg1);
                Helper::Debug::System(Helper::Format("Window focus state: %s", (*(bool*)(arg1)) ? "True" : "False"));
                Helper::Input::Reload();
                break;
        }
    });

    m_isCreate = true;

    return true;
}

bool Framework::Graphics::Window::Init() {
    if (!m_isCreate) {
        SR_ERROR("Window::Init() : window is not created!");
        return false;
    }

    if (m_isInit) {
        SR_ERROR("Window::Init() : window are already initialize!");
        return false;
    }

    Debug::Graph("Window::Init() : initializing window...");

    {
        m_thread = SR_HTYPES_NS::Thread(&Window::Thread, this);

        while (!m_isEnvInit && !m_hasErrors && !m_isClose) { } // Wait environment initialize
    }

    ret: if (!m_render->IsInit() && !m_hasErrors) goto ret;
    if (m_hasErrors)
        return false;

    if (!Types::Material::InitDefault(GetRender())) {
        SR_ERROR("Window::Init() : failed to initialize default material!");
        return false;
    }

    m_isInit = true;

    return true;
}

bool Framework::Graphics::Window::Run() {
    if (!m_isInit) {
        SR_ERROR("Window::Run() : window is not initialized!");
        return false;
    }

    if (m_isRun) {
        SR_ERROR("Window::Run() : window are already is running!");
        return false;
    }

    Debug::Graph("Window::Run() : running window...");

    m_isRun = true;

ret: if (!m_render->IsRun() && !m_hasErrors)
    goto ret;

    if (m_hasErrors)
        return false;

    SR_INFO("Window::Run() : window has been successfully running!");

    return true;
}

bool Framework::Graphics::Window::Close() {
    if (!m_isRun) {
        SR_ERROR("Window::Close() : window is not running!");
        return false;
    }

    if (m_isClose) {
        SR_ERROR("Window::Close() : window already is closed!");
        return false;
    }

    Debug::Graph("Window::Close() : close window...");

    m_isRun   = false;
    m_isClose = true;

    m_thread.TryJoin();

    return true;
}

void Framework::Graphics::Window::Thread() {
    SR_INFO("Window::Thread() : running window thread...");

    {
        waitInit:
        if (m_isInit && !m_isClose && !m_isRun && !m_hasErrors) goto waitInit;

        if (!m_hasErrors && !m_isClose)
            if (!InitEnvironment()) {
                SR_ERROR("Window::Thread() : failed to initialize render environment!");
                m_hasErrors = true;
                return;
            }

        if (!m_render->Init()) {
            SR_ERROR("Window::Thread() : failed to initialize render!");
            m_hasErrors = true;
            return;

        }

        waitRun:
        if (!m_isRun && !m_isClose && !m_hasErrors)
            goto waitRun;

        if (!m_render->Run()) {
            SR_ERROR("Window::Thread() : failed to ran render!");
            m_hasErrors = true;
            return;
        }
    }

    SR_LOG("Window::Thread() : screen size is " + m_env->GetScreenSize().ToString());

    double deltaTime = 0;
    uint32_t frames = 0;

    /// for optimization needed pipeline
    const PipeLine pipeLine = m_env->GetPipeLine();

    m_env->SetBuildState(false);

    while (IsAlive()) {
        clock_t beginFrame = clock();

        m_env->PollEvents();
        PollEvents();
        m_render->PollEvents();

        if (pipeLine == PipeLine::Vulkan) {
            DrawVulkan();
        }
        else
            DrawOpenGL();

        deltaTime += double(clock() - beginFrame) / (double) CLOCKS_PER_SEC;
        ++frames;

        if (deltaTime > 1.0) { /// every second
            std::cout << "FPS: " << frames - 1 << std::endl;
            frames = 0; deltaTime = 0;
        }
    }

    SR_GRAPH("Window::Thread() : exit from main cycle.");

    if (!m_widgetManagers.Empty()) {
        m_widgetManagers.Clear();
    }

    if (m_env->IsGUISupport()) {
        m_env->StopGUI();
        SR_GRAPH("Window::Thread() : complete stopping gui!");
    }

    SR_GTYPES_NS::Texture::FreeNoneTexture();

    if (!SyncFreeResources()) {
        SR_ERROR("Window::Thread() : failed to free resources!");
    }

    if (!m_render->Close()) {
        SR_ERROR("Window::Thread() : failed to close render!");
    }

    m_env->CloseWindow();

    Memory::MeshManager::Destroy();

    SR_INFO("Window::Thread() : stopping window thread...");

    m_isWindowClose = true;
}

bool Framework::Graphics::Window::InitEnvironment() {
    SR_GRAPH("Window::InitEnvironment() : initializing render environment...");

    SR_GRAPH("Window::InitEnvironment() : pre-initializing...");
    if (!m_env->PreInit(
            m_smoothSamples,
            "SpaRcle Engine", /// App name
            "SREngine",       /// Engine name
            ResourceManager::Instance().GetResPath().Concat("/Utilities/glslc.exe")))
    {
        SR_ERROR("Window::InitEnvironment() : failed to pre-initializing environment!");
        return false;
    }

    SR_GRAPH("Window::InitEnvironment() : creating window...");
    if (!m_env->MakeWindow(m_winName, m_size, m_fullScreen, m_resizable, m_headerEnabled)) {
        SR_ERROR("Window::InitEnvironment() : failed to creating window!");
        return false;
    }

    m_env->SetWindowIcon(Helper::ResourceManager::Instance().GetTexturesPath().Concat(m_icoPath).CStr());

    Debug::Graph("Window::InitEnvironment() : set thread context as current...");
    if (!m_env->SetContextCurrent()) {
        SR_ERROR("Window::InitEnvironment() : failed to set context!");
        return false;
    }

    SR_GRAPH("Window::InitEnvironment() : initializing the environment...");
    if (!m_env->Init(m_vsync)) {
        SR_ERROR("Window::InitEnvironment() : failed to initializing environment!");
        return false;
    }

    SR_GRAPH("Window::InitEnvironment() : post-initializing the environment...");

    if (!m_env->PostInit()) {
        Debug::Error("Window::InitEnvironment() : failed to post-initializing environment!");
        return false;
    }

    {
        SR_LOG("Window::InitEnvironment() : vendor is "   + m_env->GetVendor());
        SR_LOG("Window::InitEnvironment() : renderer is " + m_env->GetRenderer());
        SR_LOG("Window::InitEnvironment() : version is "  + m_env->GetVersion());
    }

    if (m_env->IsGUISupport()) {
        if (m_env->PreInitGUI(Helper::ResourceManager::Instance().GetResPath().Concat("Fonts/CalibriL.ttf"))) {
            ImGuiStyle & style = ImGui::GetStyle();

            if (auto&& theme = Theme::Load("Themes/Dark.xml")) {
                theme->Apply(style);
                delete theme;
            }

            const static auto iniPath = ResourceManager::Instance().GetResPath().Concat("/Configs/ImGuiEditor.config");
            ImGui::GetIO().IniFilename = iniPath.CStr();

            if (!m_env->InitGUI()) {
                SR_ERROR("Window::InitEnvironment() : failed to initializing GUI!");
                return false;
            }
        }
        else {
            SR_ERROR("Window::InitEnvironment() : failed to pre-initializing GUI!");
        }
    }

    m_isEnvInit = true;

    return true;
}

void Framework::Graphics::Window::DrawToCamera(Framework::Graphics::Camera* camera) {
    m_render->SetCurrentCamera(camera);

    camera->GetPostProcessing()->BeginGeometry();
    {
        m_render->DrawGeometry();
        m_render->DrawTransparentGeometry();
    }
    camera->GetPostProcessing()->EndGeometry();

    camera->GetPostProcessing()->BeginSkybox();
    {
        m_render->DrawSkybox();
        m_render->DrawGrid();
    }
    camera->GetPostProcessing()->EndSkybox();

    camera->GetPostProcessing()->Complete();
}

void Framework::Graphics::Window::CentralizeCursor() noexcept {
    if (m_isRun) {
        m_env->SetCursorPosition({ GetWindowSize().x / 2,  GetWindowSize().y / 2});
    }
    else {
        SR_ERROR("Window::CentralizeCursor() : the window isn't run!");
    }
}

void Framework::Graphics::Window::PollEvents() {
    // change gui enabled
    if (m_GUIEnabled.first != m_GUIEnabled.second) {
        m_env->SetBuildState(false);
        this->m_env->SetGUIEnabled(m_GUIEnabled.second);
        m_GUIEnabled.first.store(m_GUIEnabled.second);
    }

    if (m_widgetManagers.NeedFlush())
        m_widgetManagers.Flush();

    if (m_cameras.NeedFlush()) {
        for (auto&& camera : m_cameras.GetAddedElements()) {
            camera->Create(this);
            if (!camera->CompleteResize()) {
                SR_ERROR("Window::PollEvents() : failed to complete resize camera!");
            }
        }

        for (auto&& camera : m_cameras.GetDeletedElements()) {
            if (Helper::Debug::GetLevel() > Helper::Debug::Level::Low) {
                SR_LOG("Window::PoolEvents() : remove camera...");
            }

            camera->Free();

            SR_LOG("Window::PoolEvents() : the camera has been successfully released!");
        }

        m_cameras.Flush();

        m_render->SetCurrentCamera(nullptr);
        m_env->SetBuildState(false);
    }

    if (m_isNeedResize) {
        std::lock_guard<std::mutex> lock(m_mutex);

        m_env->SetWindowSize((uint32_t)m_size.x, (uint32_t)m_size.y);
        m_isNeedResize = false;
        m_size = { 0, 0 };
    }

    if (m_isNeedMove) {
        std::lock_guard<std::mutex> lock(m_mutex);

        m_env->SetWindowPosition((int)m_newWindowPos.x, (int)m_newWindowPos.y);

        this->m_isNeedMove = false;
    }
}

bool Framework::Graphics::Window::Free() {
    if (m_isClose) {
        SR_INFO("Window::Free() : free window pointer...");

        delete this;
        return true;
    }
    else
        return false;
}

void Framework::Graphics::Window::Resize(uint32_t w, uint32_t h) {
    std::lock_guard<std::mutex> lock(m_mutex);

    SR_LOG("Window::Resize() : set new window sizes: W = " + std::to_string(w) + "; H = " + std::to_string(h));

    m_size = { (int32_t)w, (int32_t)h };
    m_isNeedResize = true;
}

void Framework::Graphics::Window::CentralizeWindow() {
    SR_INFO("Window::CentralizeWindow() : wait centralize window...");

    std::lock_guard<std::mutex> lock(m_mutex);

    if (!m_env->GetBasicWindow()) {
        SR_WARN("Window::CentralizeWindow() : basic window is nullptr!");
        return;
    }

    auto scr_size = m_env->GetScreenSize();

    auto w = m_isNeedResize ? m_size.x : m_env->GetBasicWindow()->GetWidth();
    auto h = m_isNeedResize ? m_size.y : m_env->GetBasicWindow()->GetHeight();

    w = (int) (scr_size.x - (float)w) / 2;
    h = (int) (scr_size.y - (float)h) / 2;

    m_newWindowPos = { (int32_t)w, (int32_t)h };
    m_isNeedMove = true;
}

void Framework::Graphics::Window::DestroyCamera(Framework::Graphics::Camera *camera) {
    if (Helper::Debug::GetLevel() > Helper::Debug::Level::None) {
        SR_LOG("Window::RemoveCamera() : register camera to remove...");
    }

    if (!camera) {
        SR_ERROR("Window::RemoveCamera() : camera is nullptr! The application will now crash...");
        return;
    }

    m_cameras.Remove(camera);
}

void Framework::Graphics::Window::AddCamera(Framework::Graphics::Camera *camera)  {
    if (Helper::Debug::GetLevel() > Helper::Debug::Level::None) {
        SR_LOG("Window::AddCamera() : register new camera...");
    }

    m_cameras.Add(camera);
}

void Framework::Graphics::Window::BeginSync() {
    m_mutex.lock();
}

void Framework::Graphics::Window::EndSync() {
    m_mutex.unlock();
}

bool Framework::Graphics::Window::IsAlive() const {
    return m_isRun && !m_hasErrors && !m_isClose && m_env->IsWindowOpen() && !m_env->HasErrors();
}

bool Framework::Graphics::Window::SyncFreeResources() {
    Helper::Debug::System("Window::SyncFreeResources() : synchronizing resources...");

    std::atomic<bool> syncComplete(false);

    /** Ждем, пока все графические ресурсы не освободятся */
    auto thread = Helper::Types::Thread([&syncComplete, this]() {
        uint32_t syncStep = 0;
        const uint32_t maxErrStep = 100;

        ResourceManager::Instance().Synchronize(true);

        if (auto material = SR_GTYPES_NS::Material::GetDefault(); material && material->GetCountUses() == 1)
            SR_GTYPES_NS::Material::FreeDefault();

        m_render->Synchronize();

        while(!m_render->IsClean()) {
            Helper::Debug::System("Window::SyncFreeResources() : synchronizing resources (step " + std::to_string(++syncStep) + ")");

            if (auto material = SR_GTYPES_NS::Material::GetDefault(); material && material->GetCountUses() == 1)
                SR_GTYPES_NS::Material::FreeDefault();

            ResourceManager::Instance().Synchronize(true);
            m_render->Synchronize();

            if (maxErrStep == syncStep) {
                SR_ERROR("Window::SyncFreeResources() : [FATAL] resources can not be released!");
                Helper::ResourceManager::Instance().PrintMemoryDump();
                Helper::Debug::Terminate();
                break;
            }

            Helper::Types::Thread::Sleep(50);
        }

        syncComplete = true;
    });

    /** Так как некоторые ресурсы, такие как материалы, имеют вложенные ресурсы,
     * то они могут ожидать пока графический поток уберет метку использования с них */
    while (!syncComplete) {
        PollEvents();
        m_render->PollEvents();
    }

    if (auto&& material = SR_GTYPES_NS::Material::GetDefault()) {
        SRAssert2(false, "Window::SyncFreeResources() : default material was not be freed!\n\tUses count: " +
                         std::to_string(material->GetCountUses()));
    }

    thread.TryJoin();

    Helper::Debug::System("Window::SyncFreeResources() : complete synchronizing!");

    return true;
}

void Framework::Graphics::Window::Synchronize() {
ret:
    if (!m_isNeedMove && !m_isNeedResize && !m_env->IsNeedReBuild() && m_GUIEnabled.first == m_GUIEnabled.second)
        return;

    Helper::Types::Thread::Sleep(10);
    goto ret;
}

void Framework::Graphics::Window::DrawNoCamera() {
    m_env->ClearFramebuffersQueue();

    {
        m_env->ClearBuffers(0.5f, 0.5f, 0.5f, 1.f, 1.f, 1);

        m_render->CalculateAll();

        for (uint8_t i = 0; i < m_env->GetCountBuildIter(); ++i) {
            m_env->SetBuildIteration(i);

            m_env->BindFrameBuffer(0);

            m_env->BeginRender();
            {
                m_env->SetViewport();
                m_env->SetScissor();
            }
            m_env->EndRender();
        }
    }

    m_env->SetBuildState(true);
}

void Framework::Graphics::Window::RegisterWidgetManager(GUI::WidgetManager* widgetManager) {
    m_widgetManagers.Add(widgetManager);
}

void Framework::Graphics::Window::SetGUIEnabled(bool value) {
    if (value) {
        SR_LOG("Window::SetGUIEnabled() : enable gui...");
    }
    else
        SR_LOG("Window::SetGUIEnabled() : disable gui...");

    m_GUIEnabled.second = value;
}

void Framework::Graphics::Window::SetFullScreen(bool value) {
    m_env->SetFullScreen(value);
}

void Framework::Graphics::Window::DrawVulkan() {
    if (IsGUIEnabled() && m_env->IsGUISupport() && !m_env->IsWindowCollapsed()) {
        if (m_env->BeginDrawGUI()) {
            for (auto&& widgetManager : m_widgetManagers.GetElements())
                widgetManager->Draw();

            m_env->EndDrawGUI();
        }
    }

    BeginSync();

    for (auto&& camera : m_cameras.GetElements())
        camera->PoolEvents();

    if (m_env->IsNeedReBuild()) {
        if (!m_cameras.Empty()) {
            m_env->ClearFramebuffersQueue();

            m_render->SetCurrentCamera(m_cameras.Front());

            if (m_cameras.Front()->IsReady()) {
                if (m_cameras.Front()->GetPostProcessing()->BeginGeometry()) {
                    m_env->BeginRender();

                    {
                        m_env->SetViewport();
                        m_env->SetScissor();

                        m_render->DrawGeometry();
                        m_render->DrawSkybox();
                    }

                    m_env->EndRender();

                    m_cameras.Front()->GetPostProcessing()->EndGeometry();
                }
            }

            {
                m_env->ClearBuffers(0.5f, 0.5f, 0.5f, 1.f, 1.f, 1);

                if (!m_cameras.Front()->IsDirectOutput()) {
                    m_env->BindFrameBuffer(m_cameras.Front()->GetPostProcessing()->GetFinalFBO());
                    m_env->ClearBuffers();

                    m_cameras.Front()->GetPostProcessing()->Complete();

                    m_env->BeginRender();
                    {
                        m_env->SetViewport();
                        m_env->SetScissor();

                        //! Должна вызываться в том же кадровом буфере, что и Complete
                        m_cameras.Front()->GetPostProcessing()->Draw();
                    }
                    m_env->EndRender();
                }
                else
                    m_cameras.Front()->GetPostProcessing()->Complete();

                for (uint8_t i = 0; i < m_env->GetCountBuildIter(); i++) {
                    m_env->SetBuildIteration(i);

                    m_env->BindFrameBuffer(0);

                    m_env->BeginRender();
                    {
                        m_env->SetViewport();
                        m_env->SetScissor();

                        if (m_cameras.Front()->IsDirectOutput())
                            m_cameras.Front()->GetPostProcessing()->Draw();
                    }
                    m_env->EndRender();
                }
            }

            m_env->SetBuildState(true);
        }
        else
            DrawNoCamera();

        EndSync();
        return;
    }
    else
        m_render->UpdateUBOs();

    m_env->DrawFrame();

    EndSync();
}

void Framework::Graphics::Window::DrawOpenGL() {
    for (auto&& camera : m_cameras.GetElements())
        camera->PoolEvents();

    m_env->ClearBuffers();

    {
        if (m_cameras.Count() == 1) {
            if (m_cameras.Front()->IsReady()) {
                DrawToCamera(m_cameras.Front());
            }
        }
        else
            for (auto&& camera : m_cameras.GetElements()) {
                if (!camera->IsReady()) {
                    return;
                }
                DrawToCamera(camera);
            }

        if (IsGUIEnabled() && m_env->IsGUISupport()) {
            if (m_env->BeginDrawGUI()) {
                m_env->EndDrawGUI();
            }
        }
    }

    m_env->SwapBuffers();
}

