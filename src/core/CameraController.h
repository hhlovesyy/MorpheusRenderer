// src/core/CameraController.h (ÐÞ¸Äºó)
#pragma once
#include "../math/Vector.h"

// Forward declare Camera to avoid circular dependency
namespace Morpheus::Scene { class Camera; }

namespace Morpheus::Core {

    class CameraController {
    public:
        CameraController();

        // Now takes a pointer to the camera it should control
        void Update(Scene::Camera* camera, float deltaTime);

    private:
        // Controller state, not camera state
        float m_distance;
        float m_pitch;
        float m_yaw;

		Math::Vector3f m_focalPoint; // The point the camera is looking at, can be set externally
    };
}
