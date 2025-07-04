#pragma once
#include "Vector.h"
#include <cstring>
#include <cmath>
#include <cassert>

namespace Morpheus::Math {
    class Matrix4f {
    public:
        float m[4][4];

        Matrix4f() {
            memset(m, 0, sizeof(m));
        }

        float* operator[](size_t i) { return m[i]; }
        const float* operator[](size_t i) const { return m[i]; }

        Matrix4f operator*(const Matrix4f& other) const {
            Matrix4f result;
            for (int i = 0; i < 4; ++i) {
                for (int j = 0; j < 4; ++j) {
                    for (int k = 0; k < 4; ++k) {
                        result[i][j] += m[i][k] * other.m[k][j];
                    }
                }
            }
            return result;
        }

        Vector4f operator*(const Vector4f& v) const {
            Vector4f res{};
            for (int i = 0; i < 4; ++i) {
                res[i] = m[i][0] * v.x() + m[i][1] * v.y() + m[i][2] * v.z() + m[i][3] * v.w();
            }
            return res;
        }

        Matrix4f transpose() const {
            Matrix4f result;
            for (int i = 0; i < 4; ++i) {
                for (int j = 0; j < 4; ++j) {
                    result[i][j] = m[j][i];
                }
            }
            return result;
        }

        // --- 这是完整的求逆函数实现 ---
        Matrix4f inverse() const {
            Matrix4f inv;

            inv[0][0] = m[1][1] * m[2][2] * m[3][3] -
                m[1][1] * m[2][3] * m[3][2] -
                m[2][1] * m[1][2] * m[3][3] +
                m[2][1] * m[1][3] * m[3][2] +
                m[3][1] * m[1][2] * m[2][3] -
                m[3][1] * m[1][3] * m[2][2];

            inv[1][0] = -m[1][0] * m[2][2] * m[3][3] +
                m[1][0] * m[2][3] * m[3][2] +
                m[2][0] * m[1][2] * m[3][3] -
                m[2][0] * m[1][3] * m[3][2] -
                m[3][0] * m[1][2] * m[2][3] +
                m[3][0] * m[1][3] * m[2][2];

            inv[2][0] = m[1][0] * m[2][1] * m[3][3] -
                m[1][0] * m[2][3] * m[3][1] -
                m[2][0] * m[1][1] * m[3][3] +
                m[2][0] * m[1][3] * m[3][1] +
                m[3][0] * m[1][1] * m[2][3] -
                m[3][0] * m[1][3] * m[2][1];

            inv[3][0] = -m[1][0] * m[2][1] * m[3][2] +
                m[1][0] * m[2][2] * m[3][1] +
                m[2][0] * m[1][1] * m[3][2] -
                m[2][0] * m[1][2] * m[3][1] -
                m[3][0] * m[1][1] * m[2][2] +
                m[3][0] * m[1][2] * m[2][1];

            inv[0][1] = -m[0][1] * m[2][2] * m[3][3] +
                m[0][1] * m[2][3] * m[3][2] +
                m[2][1] * m[0][2] * m[3][3] -
                m[2][1] * m[0][3] * m[3][2] -
                m[3][1] * m[0][2] * m[2][3] +
                m[3][1] * m[0][3] * m[2][2];

            inv[1][1] = m[0][0] * m[2][2] * m[3][3] -
                m[0][0] * m[2][3] * m[3][2] -
                m[2][0] * m[0][2] * m[3][3] +
                m[2][0] * m[0][3] * m[3][2] +
                m[3][0] * m[0][2] * m[2][3] -
                m[3][0] * m[0][3] * m[2][2];

            inv[2][1] = -m[0][0] * m[2][1] * m[3][3] +
                m[0][0] * m[2][3] * m[3][1] +
                m[2][0] * m[0][1] * m[3][3] -
                m[2][0] * m[0][3] * m[3][1] -
                m[3][0] * m[0][1] * m[2][3] +
                m[3][0] * m[0][3] * m[2][1];

            inv[3][1] = m[0][0] * m[2][1] * m[3][2] -
                m[0][0] * m[2][2] * m[3][1] -
                m[2][0] * m[0][1] * m[3][2] +
                m[2][0] * m[0][2] * m[3][1] +
                m[3][0] * m[0][1] * m[2][2] -
                m[3][0] * m[0][2] * m[2][1];

            inv[0][2] = m[0][1] * m[1][2] * m[3][3] -
                m[0][1] * m[1][3] * m[3][2] -
                m[1][1] * m[0][2] * m[3][3] +
                m[1][1] * m[0][3] * m[3][2] +
                m[3][1] * m[0][2] * m[1][3] -
                m[3][1] * m[0][3] * m[1][2];

            inv[1][2] = -m[0][0] * m[1][2] * m[3][3] +
                m[0][0] * m[1][3] * m[3][2] +
                m[1][0] * m[0][2] * m[3][3] -
                m[1][0] * m[0][3] * m[3][2] -
                m[3][0] * m[0][2] * m[1][3] +
                m[3][0] * m[0][3] * m[1][2];

            inv[2][2] = m[0][0] * m[1][1] * m[3][3] -
                m[0][0] * m[1][3] * m[3][1] -
                m[1][0] * m[0][1] * m[3][3] +
                m[1][0] * m[0][3] * m[3][1] +
                m[3][0] * m[0][1] * m[1][3] -
                m[3][0] * m[0][3] * m[1][1];

            inv[3][2] = -m[0][0] * m[1][1] * m[3][2] +
                m[0][0] * m[1][2] * m[3][1] +
                m[1][0] * m[0][1] * m[3][2] -
                m[1][0] * m[0][2] * m[3][1] -
                m[3][0] * m[0][1] * m[1][2] +
                m[3][0] * m[0][2] * m[1][1];

            inv[0][3] = -m[0][1] * m[1][2] * m[2][3] +
                m[0][1] * m[1][3] * m[2][2] +
                m[1][1] * m[0][2] * m[2][3] -
                m[1][1] * m[0][3] * m[2][2] -
                m[2][1] * m[0][2] * m[1][3] +
                m[2][1] * m[0][3] * m[1][2];

            inv[1][3] = m[0][0] * m[1][2] * m[2][3] -
                m[0][0] * m[1][3] * m[2][2] -
                m[1][0] * m[0][2] * m[2][3] +
                m[1][0] * m[0][3] * m[2][2] +
                m[2][0] * m[0][2] * m[1][3] -
                m[2][0] * m[0][3] * m[1][2];

            inv[2][3] = -m[0][0] * m[1][1] * m[2][3] +
                m[0][0] * m[1][3] * m[2][1] +
                m[1][0] * m[0][1] * m[2][3] -
                m[1][0] * m[0][3] * m[2][1] -
                m[2][0] * m[0][1] * m[1][3] +
                m[2][0] * m[0][3] * m[1][1];

            inv[3][3] = m[0][0] * m[1][1] * m[2][2] -
                m[0][0] * m[1][2] * m[2][1] -
                m[1][0] * m[0][1] * m[2][2] +
                m[1][0] * m[0][2] * m[2][1] +
                m[2][0] * m[0][1] * m[1][2] -
                m[2][0] * m[0][2] * m[1][1];

            float det = m[0][0] * inv[0][0] + m[0][1] * inv[1][0] + m[0][2] * inv[2][0] + m[0][3] * inv[3][0];

            if (std::abs(det) < 1e-6) {
                // Matrix is non-invertible (or singular).
                // Returning an identity matrix is a safe fallback.
                return Matrix4f::Identity();
            }

            float one_over_det = 1.0f / det;
            Matrix4f result;
            for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 4; j++) {
                    result[i][j] = inv[i][j] * one_over_det;
                }
            }
            return result;
        }

        static Matrix4f Identity() {
            Matrix4f mat;
            mat.m[0][0] = mat.m[1][1] = mat.m[2][2] = mat.m[3][3] = 1.0f;
            return mat;
        }

        static Matrix4f Perspective(float fovy_rad, float aspect, float zNear, float zFar) {
            assert(aspect > 0);
            assert(zFar > zNear);

            Matrix4f mat{};
            float const tanHalfFovy = tan(fovy_rad / 2.0f);
            mat.m[0][0] = 1.0f / (aspect * tanHalfFovy);
            mat.m[1][1] = 1.0f / (tanHalfFovy);
            mat.m[2][2] = -(zFar + zNear) / (zFar - zNear);
            mat.m[2][3] = -(2.0f * zFar * zNear) / (zFar - zNear);
            mat.m[3][2] = -1.0f;
            return mat;
        }

        static Matrix4f Translate(const Vector3f& v) {
            Matrix4f result = Identity();
            result[0][3] = v.x();
            result[1][3] = v.y();
            result[2][3] = v.z();
            return result;
        }

        static Matrix4f Scale(const Vector3f& v) {
            Matrix4f result = Identity();
            result[0][0] = v.x();
            result[1][1] = v.y();
            result[2][2] = v.z();
            return result;
        }

        static Matrix4f Rotate(float angle_rad, const Vector3f& axis) {
            Vector3f a = normalize(axis);
            float s = std::sin(angle_rad);
            float c = std::cos(angle_rad);
            float oc = 1.0f - c;

            Matrix4f result;
            result[0][0] = oc * a.x() * a.x() + c;
            result[0][1] = oc * a.x() * a.y() - a.z() * s;
            result[0][2] = oc * a.z() * a.x() + a.y() * s;
            result[0][3] = 0.0f;

            result[1][0] = oc * a.x() * a.y() + a.z() * s;
            result[1][1] = oc * a.y() * a.y() + c;
            result[1][2] = oc * a.y() * a.z() - a.x() * s;
            result[1][3] = 0.0f;

            result[2][0] = oc * a.z() * a.x() - a.y() * s;
            result[2][1] = oc * a.y() * a.z() + a.x() * s;
            result[2][2] = oc * a.z() * a.z() + c;
            result[2][3] = 0.0f;

            result[3][0] = 0.0f;
            result[3][1] = 0.0f;
            result[3][2] = 0.0f;
            result[3][3] = 1.0f;
            return result;
        }
    };
}
