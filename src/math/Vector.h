#pragma once
#include <cmath>
#include <iostream>
#include <cassert> // For assert

namespace Morpheus::Math {

    // --- 我们将使用一个统一的模板，不再对 vec<3> 和 vec<4> 进行完全特化 ---
    // --- 这样可以共享所有运算符重载，避免代码重复 ---
    template<size_t N, typename T>
    struct vec {
        T data[N];

        T& operator[](size_t i) {
            assert(i < N);
            return data[i];
        }
        const T& operator[](size_t i) const {
            assert(i < N);
            return data[i];
        }

        // 向量 + 向量
        vec<N, T> operator+(const vec<N, T>& other) const {
            vec<N, T> result;
            for (size_t i = 0; i < N; ++i) {
                result[i] = data[i] + other[i];
            }
            return result;
        }

        // 向量 - 向量
        vec<N, T> operator-(const vec<N, T>& other) const {
            vec<N, T> result;
            for (size_t i = 0; i < N; ++i) {
                result[i] = data[i] - other[i];
            }
            return result;
        }

        // 向量 * 标量
        vec<N, T> operator*(T scalar) const {
            vec<N, T> result;
            for (size_t i = 0; i < N; ++i) {
                result[i] = data[i] * scalar;
            }
            return result;
        }

        // 向量 / 标量
        vec<N, T> operator/(T scalar) const {
            vec<N, T> result;
            for (size_t i = 0; i < N; ++i) {
                result[i] = data[i] / scalar;
            }
            return result;
        }

        // --- 为了方便，我们可以添加 .x, .y, .z, .w 访问器 ---
        // --- 使用 if constexpr (C++17) 来确保只在维度足够时才编译这些函数 ---
        T& x() { if constexpr (N > 0) return data[0]; }
        const T& x() const { if constexpr (N > 0) return data[0]; }
        T& y() { if constexpr (N > 1) return data[1]; }
        const T& y() const { if constexpr (N > 1) return data[1]; }
        T& z() { if constexpr (N > 2) return data[2]; }
        const T& z() const { if constexpr (N > 2) return data[2]; }
        T& w() { if constexpr (N > 3) return data[3]; }
        const T& w() const { if constexpr (N > 3) return data[3]; }
    };

    // 向量类型别名
    using Vector2f = vec<2, float>;
    using Vector3f = vec<3, float>;
    using Vector4f = vec<4, float>;

    // 叉积函数保持不变，但要确保它能访问 .x, .y, .z
    inline Vector3f cross(const Vector3f& a, const Vector3f& b) {
        return { a.y() * b.z() - a.z() * b.y(),
                 a.z() * b.x() - a.x() * b.z(),
                 a.x() * b.y() - a.y() * b.x() };
    }
}
