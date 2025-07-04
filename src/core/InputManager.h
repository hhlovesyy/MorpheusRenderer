// src/core/InputManager.h (���ļ�)
#pragma once
#include "../math/Vector.h"
#include <SDL.h>
#include <array>

namespace Morpheus::Core {
    class InputManager {
    public:
        // ɾ����������͸�ֵ
        InputManager(const InputManager&) = delete;
        InputManager& operator=(const InputManager&) = delete;

        static InputManager& Get();

        void PollEvents();
        bool IsQuitRequested() const;

        bool IsKeyPressed(SDL_Scancode key) const;
        bool IsMouseButtonDown(int button) const; // 1: left, 2: middle, 3: right

        Math::vec<2, int> GetMousePosition() const;
        Math::vec<2, int> GetMouseDelta() const;
        int GetMouseScrollDelta() const;

    private:
        InputManager(); // ˽�й��캯��

        const Uint8* m_keyboardState;
        std::array<bool, 5> m_mouseButtonState; // Up to 5 buttons
        Math::vec<2, int> m_mousePos;
        Math::vec<2, int> m_mouseDelta;
        int m_mouseScrollDelta;
        bool m_quitRequested;
    };
}
