// src/core/CameraController.cpp (���ļ�)
#include "CameraController.h"
#include "../scene/Camera.h" // Include the full header here
#include "InputManager.h"
#include <algorithm>

namespace Morpheus::Core {

    CameraController::CameraController()
        : m_distance(5.0f),
        m_pitch(0.0f),
        m_yaw(0.0f)
    {
        // No need to calculate matrix here anymore
		m_focalPoint = { 0.0f, 0.0f, 0.0f }; // Ĭ�Ͻ�����ԭ��
    }

    void CameraController::Update(Scene::Camera* camera, float deltaTime) {
        if (!camera) return;
        auto& input = InputManager::Get();

        // ��� Alt ���Ƿ񱻰���
        bool altPressed = input.IsKeyPressed(SDL_SCANCODE_LALT) || input.IsKeyPressed(SDL_SCANCODE_RALT);


        if (altPressed) {
            // Alt + Left Mouse Button: Orbit (��ת)
            if (input.IsMouseButtonDown(1)) { // 1 is left mouse button
                auto delta = input.GetMouseDelta();
                m_yaw -= delta.x() * 0.005f;
                m_pitch += delta.y() * 0.005f;
                // ����pitch�Ƕȣ���ֹ����������;�ͷ��ת
                m_pitch = std::clamp(m_pitch, -1.5f, 1.5f);
            }

            // Alt + Middle Mouse Button: Pan (ƽ��)
            if (input.IsMouseButtonDown(2)) { // 2 is middle mouse button
                auto delta = input.GetMouseDelta();

                // ������Ҫ���������ǰ�ĳ���������ȷ�� right �� up ����
                // ��������߼��� RecalculateViewMatrix ���У�����ֱ������ m_viewMatrix ����������õ�
                // һ�����򵥵ķ�����ֱ�Ӵ� yaw/pitch ����
                float cos_p = cos(m_pitch);
                float sin_p = sin(m_pitch);
                float cos_y = cos(m_yaw);
                float sin_y = sin(m_yaw);

                // �������ϵ�� right ��������������ϵ�еı�ʾ
                Math::Vector3f right = { cos_y, 0, -sin_y };
                // �������ϵ�� up ��������������ϵ�еı�ʾ
                Math::Vector3f up = { sin_p * sin_y, cos_p, sin_p * cos_y };

                // �������λ�ƺ�����������ƶ�����
                m_focalPoint.x() -= right.x() * delta.x() * 0.01f - up.x() * delta.y() * 0.01f;
                m_focalPoint.y() -= right.y() * delta.x() * 0.01f - up.y() * delta.y() * 0.01f;
                m_focalPoint.z() -= right.z() * delta.x() * 0.01f - up.z() * delta.y() * 0.01f;
            }

            // Alt + Right Mouse Button: Zoom (����)
            if (input.IsMouseButtonDown(3)) { // 3 is right mouse button
                auto delta = input.GetMouseDelta();
                // ���»������ƶ��������������ţ����������� y ��
                m_distance -= delta.y() * 0.05f;
            }
        }

        // Mouse Wheel: Zoom (�����Ƿ�סAlt����Ч)
        int scrollDelta = input.GetMouseScrollDelta();
        if (scrollDelta != 0) {
            m_distance -= scrollDelta * 0.5f;
        }

        // ȷ�����벻���ɸ������С
        m_distance = std::max(0.1f, m_distance);

        // ÿ�θ��º����¼�����ͼ����
        Math::Vector3f newPosition;
        newPosition.x() = m_focalPoint.x() + m_distance * cos(m_pitch) * sin(m_yaw);
        newPosition.y() = m_focalPoint.y() + m_distance * sin(m_pitch);
        newPosition.z() = m_focalPoint.z() + m_distance * cos(m_pitch) * cos(m_yaw);

        // Update the actual camera object
        camera->SetPosition(newPosition);
        camera->SetFocalPoint(m_focalPoint);
        camera->UpdateViewMatrix(); // Tell the camera to recompute its view matrix
    }
}
