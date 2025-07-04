#include "Application.h"
#include <stdexcept>
#include <SDL.h>
#include "../renderer/Renderer.h"
#include "../renderer/Mesh.h"
#include "../scene/Scene.h"
#include "InputManager.h"  
#include "CameraController.h" // <--- 新增
#include "../renderer/shaders/UnlitShader.h" // <--- 包含我们要注册的 Shader

// --- 在所有 Application 成员函数之前，定义一个辅助函数 ---
void RegisterShaders() {
    Morpheus::Scene::Scene::RegisterShader("Unlit", []() {
        return std::make_shared<Morpheus::Renderer::UnlitShader>();
        });
    // 未来在这里注册 PBR Shader 等
    // Morpheus::Scene::Scene::RegisterShader("PBR", []() { 
    //     return std::make_shared<Morpheus::Renderer::PBRShader>(); 
    // });
}

namespace Morpheus::Core {

    Application::Application(const std::string& title, int width, int height)
        : m_title(title), m_width(width), m_height(height) {
        // --- 在所有初始化之前，先注册 Shaders ---
        RegisterShaders();
        Initialize();
    }

    Application::~Application() {
        Shutdown();
    }

    void Application::Initialize() {
       

        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            throw std::runtime_error("Could not initialize SDL");
        }
        m_window = SDL_CreateWindow(m_title.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, m_width, m_height, SDL_WINDOW_SHOWN);
        if (!m_window) {
            throw std::runtime_error("Could not create window");
        }
        m_sdlRenderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED);
        m_screenTexture = SDL_CreateTexture(m_sdlRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, m_width, m_height);

        m_morpheusRenderer = std::make_unique<Renderer::Renderer>(m_width, m_height);
        m_cameraController = std::make_unique<CameraController>(); // <--- 新增
        // --- 核心修改：加载整个场景 ---
        try {
            m_scene = std::make_unique<Scene::Scene>(Scene::Scene::Load("assets/scenes/test_scene.json"));
            // 初始化相机的投影矩阵
            m_scene->GetCamera().SetPerspective(45.0f, (float)m_width / (float)m_height, 0.1f, 100.0f);

        }
        catch (const std::exception& e) {
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Scene Load Error", e.what(), m_window);
            m_isRunning = false;
        }
    }

    void Application::Run() {
        while (m_isRunning) {
            // 用InputManager替换旧的事件处理
            InputManager::Get().PollEvents();
            if (InputManager::Get().IsQuitRequested()) {
                m_isRunning = false;
            }
            Update(0.016f); // Assume 60fps for now
            RenderFrame();
        }
    }

    void Application::HandleEvents() {
        SDL_Event e;
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                m_isRunning = false;
            }
        }
    }

    void Application::Update(float deltaTime) {
        if (m_scene) {
            m_cameraController->Update(&m_scene->GetCamera(), deltaTime);
        }
    }

    void Application::RenderFrame() {
        if (!m_scene) return;

        // --- 核心修改：将整个场景传递给渲染器 ---
        m_morpheusRenderer->Render(*m_scene);

        // 3. 后续的 "blit" 代码保持不变
        auto framebuffer = m_morpheusRenderer->GetFramebuffer();
        if (framebuffer) { // 最好加个空指针检查
            SDL_UpdateTexture(m_screenTexture, NULL, framebuffer->GetPixelData(), framebuffer->GetWidth() * sizeof(uint32_t));
            SDL_RenderClear(m_sdlRenderer);
            SDL_RenderCopy(m_sdlRenderer, m_screenTexture, NULL, NULL);
            SDL_RenderPresent(m_sdlRenderer);
        }
    }

    void Application::Shutdown() {
        SDL_DestroyTexture(m_screenTexture);
        SDL_DestroyRenderer(m_sdlRenderer);
        SDL_DestroyWindow(m_window);
        SDL_Quit();
    }
}
