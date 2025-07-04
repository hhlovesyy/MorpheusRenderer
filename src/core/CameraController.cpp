// src/core/CameraController.cpp (新文件)
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
		m_focalPoint = { 0.0f, 0.0f, 0.0f }; // 默认焦点在原点
    }

    void CameraController::Update(Scene::Camera* camera, float deltaTime) {
        if (!camera) return;
        auto& input = InputManager::Get();

        // 检查 Alt 键是否被按下
        bool altPressed = input.IsKeyPressed(SDL_SCANCODE_LALT) || input.IsKeyPressed(SDL_SCANCODE_RALT);


        if (altPressed) {
            // Alt + Left Mouse Button: Orbit (旋转)
            if (input.IsMouseButtonDown(1)) { // 1 is left mouse button
                auto delta = input.GetMouseDelta();
                m_yaw -= delta.x() * 0.005f;
                m_pitch += delta.y() * 0.005f;
                // 限制pitch角度，防止万向节死锁和镜头翻转
                m_pitch = std::clamp(m_pitch, -1.5f, 1.5f);
            }

            // Alt + Middle Mouse Button: Pan (平移)
            if (input.IsMouseButtonDown(2)) { // 2 is middle mouse button
                auto delta = input.GetMouseDelta();

                // 我们需要根据相机当前的朝向计算出正确的 right 和 up 向量
                // 这个计算逻辑在 RecalculateViewMatrix 中有，我们直接利用 m_viewMatrix 的逆矩阵来得到
                // 一个更简单的方法是直接从 yaw/pitch 计算
                float cos_p = cos(m_pitch);
                float sin_p = sin(m_pitch);
                float cos_y = cos(m_yaw);
                float sin_y = sin(m_yaw);

                // 相机坐标系的 right 向量在世界坐标系中的表示
                Math::Vector3f right = { cos_y, 0, -sin_y };
                // 相机坐标系的 up 向量在世界坐标系中的表示
                Math::Vector3f up = { sin_p * sin_y, cos_p, sin_p * cos_y };

                // 根据鼠标位移和相机朝向来移动焦点
                m_focalPoint.x() -= right.x() * delta.x() * 0.01f - up.x() * delta.y() * 0.01f;
                m_focalPoint.y() -= right.y() * delta.x() * 0.01f - up.y() * delta.y() * 0.01f;
                m_focalPoint.z() -= right.z() * delta.x() * 0.01f - up.z() * delta.y() * 0.01f;
            }

            // Alt + Right Mouse Button: Zoom (缩放)
            if (input.IsMouseButtonDown(3)) { // 3 is right mouse button
                auto delta = input.GetMouseDelta();
                // 上下或左右移动都可以用于缩放，这里我们用 y 轴
                m_distance -= delta.y() * 0.05f;
            }
        }

        // Mouse Wheel: Zoom (无论是否按住Alt都生效)
        int scrollDelta = input.GetMouseScrollDelta();
        if (scrollDelta != 0) {
            m_distance -= scrollDelta * 0.5f;
        }

        // 确保距离不会变成负数或过小
        m_distance = std::max(0.1f, m_distance);

        // 每次更新后都重新计算视图矩阵
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
