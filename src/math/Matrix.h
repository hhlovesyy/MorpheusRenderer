// src/math/Matrix.h
#pragma once
#include "Vector.h"
#include <cstring> // for memset
#include <cmath>   // for tan

namespace Morpheus::Math {
    class Matrix4f {
    public:
        float m[4][4];

        Matrix4f() {
            memset(m, 0, sizeof(m));
        }

        // --- 这是解决C++错误的关键部分 ---
        Matrix4f operator*(const Matrix4f& other) const {
            Matrix4f result;
            for (int i = 0; i < 4; ++i) {
                for (int j = 0; j < 4; ++j) {
                    // result.m[i][j] is initially 0 because of memset in constructor
                    for (int k = 0; k < 4; ++k) {
                        result.m[i][j] += m[i][k] * other.m[k][j];
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

        static Matrix4f Identity() {
            Matrix4f mat;
            mat.m[0][0] = mat.m[1][1] = mat.m[2][2] = mat.m[3][3] = 1.0f;
            return mat;
        }

        //static Matrix4f LookAt(const Vector3f& eye, const Vector3f& center, const Vector3f& up) {
        //    // 我们暂时仍然用一个简单的视图
        //    Matrix4f mat = Identity();
        //    mat.m[2][3] = -3; // 稍微向后移动相机
        //    return mat;
        //}

        static Matrix4f Perspective(float fovy_rad, float aspect, float zNear, float zFar) {
            Matrix4f mat{}; // Initialize to zero
            float const tanHalfFovy = tan(fovy_rad / 2.0f);
            mat.m[0][0] = 1.0f / (aspect * tanHalfFovy);
            mat.m[1][1] = 1.0f / (tanHalfFovy);
            mat.m[2][2] = -(zFar + zNear) / (zFar - zNear);
            mat.m[2][3] = -(2.0f * zFar * zNear) / (zFar - zNear);
            mat.m[3][2] = -1.0f;
            return mat;
        }
    };
}