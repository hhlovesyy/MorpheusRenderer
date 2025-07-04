// src/scene/Camera.cpp (ÐÂÎÄ¼þ)
#include "Camera.h"
#include <cmath>

namespace Morpheus::Scene {

    Camera::Camera()
        : m_position({ 0.0f, 0.0f, 5.0f }),
        m_focalPoint({ 0.0f, 0.0f, 0.0f }),
        m_upVector({ 0.0f, 1.0f, 0.0f })
    {
        UpdateViewMatrix();
        m_projectionMatrix = Math::Matrix4f::Identity();
    }

    void Camera::SetPerspective(float fov_degrees, float aspect, float zNear, float zFar) {
        float fov_rad = fov_degrees * 3.14159265f / 180.0f;
        m_projectionMatrix = Math::Matrix4f::Perspective(fov_rad, aspect, zNear, zFar);
    }

    void Camera::SetPosition(const Math::Vector3f& position) {
        m_position = position;
    }

    void Camera::SetFocalPoint(const Math::Vector3f& focalPoint) {
        m_focalPoint = focalPoint;
    }

    void Camera::UpdateViewMatrix() {
        // A proper LookAt implementation
        Math::Vector3f zaxis = { m_position.x() - m_focalPoint.x(), m_position.y() - m_focalPoint.y(), m_position.z() - m_focalPoint.z() };
        float z_len = sqrt(zaxis.x() * zaxis.x() + zaxis.y() * zaxis.y() + zaxis.z() * zaxis.z());
        zaxis = zaxis / z_len;

        Math::Vector3f xaxis = Math::cross(m_upVector, zaxis);
        float x_len = sqrt(xaxis.x() * xaxis.x() + xaxis.y() * xaxis.y() + xaxis.z() * xaxis.z());
        xaxis = xaxis / x_len;

        Math::Vector3f yaxis = Math::cross(zaxis, xaxis);

        m_viewMatrix = Math::Matrix4f::Identity();
        m_viewMatrix.m[0][0] = xaxis.x(); m_viewMatrix.m[0][1] = xaxis.y(); m_viewMatrix.m[0][2] = xaxis.z();
        m_viewMatrix.m[1][0] = yaxis.x(); m_viewMatrix.m[1][1] = yaxis.y(); m_viewMatrix.m[1][2] = yaxis.z();
        m_viewMatrix.m[2][0] = zaxis.x(); m_viewMatrix.m[2][1] = zaxis.y(); m_viewMatrix.m[2][2] = zaxis.z();

        m_viewMatrix.m[0][3] = -(xaxis.x() * m_position.x() + xaxis.y() * m_position.y() + xaxis.z() * m_position.z());
        m_viewMatrix.m[1][3] = -(yaxis.x() * m_position.x() + yaxis.y() * m_position.y() + yaxis.z() * m_position.z());
        m_viewMatrix.m[2][3] = -(zaxis.x() * m_position.x() + zaxis.y() * m_position.y() + zaxis.z() * m_position.z());
    }

    const Math::Matrix4f& Camera::GetViewMatrix() const { return m_viewMatrix; }
    const Math::Matrix4f& Camera::GetProjectionMatrix() const { return m_projectionMatrix; }
    const Math::Vector3f& Camera::GetPosition() const { return m_position; }
}
