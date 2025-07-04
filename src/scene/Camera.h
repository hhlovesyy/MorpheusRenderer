// src/scene/Camera.h (ÐÂÎÄ¼þ)
#pragma once
#include "../math/Matrix.h"
#include "../math/Vector.h"

namespace Morpheus::Scene {

    class Camera {
    public:
        Camera();

        // --- Setters for camera properties ---
        void SetPerspective(float fov_degrees, float aspect, float zNear, float zFar);
        void SetPosition(const Math::Vector3f& position);
        void SetFocalPoint(const Math::Vector3f& focalPoint);

        // --- Getters ---
        const Math::Matrix4f& GetViewMatrix() const;
        const Math::Matrix4f& GetProjectionMatrix() const;
        const Math::Vector3f& GetPosition() const;

        // Called by CameraController to update the view matrix
        void UpdateViewMatrix();

    private:
        Math::Matrix4f m_viewMatrix;
        Math::Matrix4f m_projectionMatrix;

        Math::Vector3f m_position;
        Math::Vector3f m_focalPoint;
        Math::Vector3f m_upVector;
    };
}
