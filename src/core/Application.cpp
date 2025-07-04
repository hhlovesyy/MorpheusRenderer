#include "Application.h"
#include <stdexcept>
#include <SDL.h>
#include "../renderer/Renderer.h"
#include "../renderer/Mesh.h"
#include "../scene/Scene.h"
#include "InputManager.h"  
#include "CameraController.h" // <--- 新增
#include "../renderer/shaders/UnlitShader.h" // <--- 包含我们要注册的 Shader
#include "../renderer/shaders/BlinnPhongShader.h" // <--- 包含 BlinnPhong Shader

// --- 在所有 Application 成员函数之前，定义一个辅助函数 ---
void RegisterShaders() {
    Morpheus::Scene::Scene::RegisterShader("Unlit", []() {
        return std::make_shared<Morpheus::Renderer::UnlitShader>();
        });
    // --- 新增 Blinn-Phong 的注册 ---
    Morpheus::Scene::Scene::RegisterShader("BlinnPhong", []() {
        return std::make_shared<Morpheus::Renderer::BlinnPhongShader>();
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
        // --- 初始化计时器 ---
        uint64_t last_counter = SDL_GetPerformanceCounter();

        while (m_isRunning) {
            // --- 计算 DeltaTime ---
            uint64_t now = SDL_GetPerformanceCounter();
            float deltaTime = (float)(now - last_counter) / (float)SDL_GetPerformanceFrequency();
            last_counter = now;

            // --- 更新帧率计数器 ---
            m_frameCounter++;
            m_fpsTimer += deltaTime;
            if (m_fpsTimer >= 1.0f) { // 每秒更新一次
                m_fps = (float)m_frameCounter / m_fpsTimer;
                m_frameTime = (m_fpsTimer / (float)m_frameCounter) * 1000.0f; // 转换为毫秒

                // 更新窗口标题
                std::string title = m_title + " - FPS: " + std::to_string((int)m_fps) +
                    " | Frametime: " + std::to_string(m_frameTime) + " ms";
                SDL_SetWindowTitle(m_window, title.c_str());

                // 重置计数器
                m_frameCounter = 0;
                m_fpsTimer = 0.0f;
            }

            // --- 主循环的其他部分 ---
            InputManager::Get().PollEvents();
            if (InputManager::Get().IsQuitRequested()) {
                m_isRunning = false;
            }

            Update(deltaTime); // <--- 将计算出的 deltaTime 传递给 Update
            RenderFrame();
        }
    }

    void Application::Update(float deltaTime) {
        if (m_scene) {
            // 将 deltaTime 传递给控制器，用于实现与帧率无关的移动
            m_cameraController->Update(&m_scene->GetCamera(), deltaTime);
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
