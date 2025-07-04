#pragma once
#include <string>
#include <memory>
#include "../scene/Camera.h"
#include "../scene/Scene.h"

// Forward declarations to avoid including SDL headers in our header
struct SDL_Window;
struct SDL_Renderer;
struct SDL_Texture;
namespace Morpheus::Core { class CameraController; class InputManager; }
namespace Morpheus::Renderer { class Renderer; class Mesh; }
namespace Morpheus::Scene { class Camera; class Scene; } // <--- 新增

namespace Morpheus::Core {
    class Application {
    public:
        Application(const std::string& title, int width, int height);
        ~Application();
        void Run();
    private:
        void Initialize();
        void HandleEvents();
        void Update(float deltaTime);
        void RenderFrame();
        void Shutdown();

        bool m_isRunning = true;
        int m_width, m_height;
        std::string m_title;

        SDL_Window* m_window = nullptr;
        SDL_Renderer* m_sdlRenderer = nullptr;
        SDL_Texture* m_screenTexture = nullptr;

        std::unique_ptr<Renderer::Renderer> m_morpheusRenderer;
        std::unique_ptr<Scene::Scene> m_scene;
        std::unique_ptr<CameraController> m_cameraController; // <--- 新增
    };
}
