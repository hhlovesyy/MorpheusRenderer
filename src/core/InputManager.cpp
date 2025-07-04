// src/core/InputManager.cpp (新文件)
#include "InputManager.h"

namespace Morpheus::Core {
    // --- 这里是 Get() 的实现 ---
    InputManager& InputManager::Get() {
        static InputManager instance; // Meyers' Singleton
        return instance;
    }

    // --- 这里是构造函数的实现 ---
    InputManager::InputManager()
        : m_quitRequested(false),
        m_mouseScrollDelta(0),
        m_keyboardState(SDL_GetKeyboardState(nullptr))
    {
        m_mouseButtonState.fill(false);
        m_mousePos = { 0, 0 };
        m_mouseDelta = { 0, 0 };
    }

    // --- 这里是 PollEvents 的实现 ---
    void InputManager::PollEvents() {
        // Reset per-frame state
        m_mouseDelta = { 0, 0 };
        m_mouseScrollDelta = 0;

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
                m_quitRequested = true;
                break;
            case SDL_MOUSEMOTION:
                m_mouseDelta.x() = event.motion.xrel;
                m_mouseDelta.y() = event.motion.yrel;
                break;
            case SDL_MOUSEWHEEL:
                m_mouseScrollDelta = event.wheel.y;
                break;
            }
        }

        // Update mouse position and button state after polling all events
        int x, y;
        Uint32 mouseState = SDL_GetMouseState(&x, &y);
        m_mousePos = { x, y };
        for (int i = 0; i < 5; ++i) {
            m_mouseButtonState[i] = (mouseState & SDL_BUTTON(i + 1));
        }
    }

    // --- 这里是其他 getter 函数的实现 ---
    bool InputManager::IsQuitRequested() const { return m_quitRequested; }
    bool InputManager::IsKeyPressed(SDL_Scancode key) const { return m_keyboardState[key]; }
    bool InputManager::IsMouseButtonDown(int button) const { return m_mouseButtonState[button - 1]; }
    Math::vec<2, int> InputManager::GetMousePosition() const { return m_mousePos; }
    Math::vec<2, int> InputManager::GetMouseDelta() const { return m_mouseDelta; }
    int InputManager::GetMouseScrollDelta() const { return m_mouseScrollDelta; }
}
