#pragma once
#include "Vector.h"
#include "Matrix.h"

namespace Morpheus::Math {
    class Quaternion {
    public:
        float x, y, z, w;

        Quaternion(float w = 1.f, float x = 0.f, float y = 0.f, float z = 0.f)
            : w(w), x(x), y(y), z(z) {
        }

        // ����ǹ���
        Quaternion(const Vector3f& axis, float angle_rad) {
            float s = std::sin(angle_rad / 2.0f);
            Vector3f a = normalize(axis);
            x = a.x() * s;
            y = a.y() * s;
            z = a.z() * s;
            w = std::cos(angle_rad / 2.0f);
        }

        Matrix4f to_matrix() const {
            Matrix4f mat = Matrix4f::Identity();
            // ... (��Ԫ��ת��ת�����ʵ��) ...
            return mat;
        }

        // ... (��Ԫ���˷�����һ����) ...
    };
}
