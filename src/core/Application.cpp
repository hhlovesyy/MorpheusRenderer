#include "Application.h"
#include <stdexcept>
#include <SDL.h>
#include "../renderer/Renderer.h"
#include "../renderer/Mesh.h"
#include "../scene/Scene.h"
#include "InputManager.h"  
#include "CameraController.h" // <--- ����
#include "../renderer/shaders/UnlitShader.h" // <--- ��������Ҫע��� Shader

// --- ������ Application ��Ա����֮ǰ������һ���������� ---
void RegisterShaders() {
    Morpheus::Scene::Scene::RegisterShader("Unlit", []() {
        return std::make_shared<Morpheus::Renderer::UnlitShader>();
        });
    // δ��������ע�� PBR Shader ��
    // Morpheus::Scene::Scene::RegisterShader("PBR", []() { 
    //     return std::make_shared<Morpheus::Renderer::PBRShader>(); 
    // });
}

namespace Morpheus::Core {

    Application::Application(const std::string& title, int width, int height)
        : m_title(title), m_width(width), m_height(height) {
        // --- �����г�ʼ��֮ǰ����ע�� Shaders ---
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
        m_cameraController = std::make_unique<CameraController>(); // <--- ����
        // --- �����޸ģ������������� ---
        try {
            m_scene = std::make_unique<Scene::Scene>(Scene::Scene::Load("assets/scenes/test_scene.json"));
            // ��ʼ�������ͶӰ����
            m_scene->GetCamera().SetPerspective(45.0f, (float)m_width / (float)m_height, 0.1f, 100.0f);

        }
        catch (const std::exception& e) {
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Scene Load Error", e.what(), m_window);
            m_isRunning = false;
        }
    }

    void Application::Run() {
        while (m_isRunning) {
            // ��InputManager�滻�ɵ��¼�����
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

        // --- �����޸ģ��������������ݸ���Ⱦ�� ---
        m_morpheusRenderer->Render(*m_scene);

        // 3. ������ "blit" ���뱣�ֲ���
        auto framebuffer = m_morpheusRenderer->GetFramebuffer();
        if (framebuffer) { // ��üӸ���ָ����
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
