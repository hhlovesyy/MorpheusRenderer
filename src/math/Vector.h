#pragma once
#include <cmath>
#include <iostream>
#include <cassert> // For assert

namespace Morpheus::Math {

    // --- ���ǽ�ʹ��һ��ͳһ��ģ�壬���ٶ� vec<3> �� vec<4> ������ȫ�ػ� ---
    // --- �������Թ���������������أ���������ظ� ---
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

        //ȡ��:-
        vec<N, T> operator-() const {
            vec<N, T> result;
            for (size_t i = 0; i < N; ++i) {
                result[i] = -data[i];
            }
            return result;
		}

        // ���� + ����
        vec<N, T> operator+(const vec<N, T>& other) const {
            vec<N, T> result;
            for (size_t i = 0; i < N; ++i) {
                result[i] = data[i] + other[i];
            }
            return result;
        }

        // ���� - ����
        vec<N, T> operator-(const vec<N, T>& other) const {
            vec<N, T> result;
            for (size_t i = 0; i < N; ++i) {
                result[i] = data[i] - other[i];
            }
            return result;
        }

        // ���� * ����
        vec<N, T> operator*(T scalar) const {
            vec<N, T> result;
            for (size_t i = 0; i < N; ++i) {
                result[i] = data[i] * scalar;
            }
            return result;
        }

        // ���� / ����
        vec<N, T> operator/(T scalar) const {
            vec<N, T> result;
            for (size_t i = 0; i < N; ++i) {
                result[i] = data[i] / scalar;
            }
            return result;
        }

        // --- �����ĳ�Ա���� ---
        T length_squared() const {
            T result = 0;
            for (size_t i = 0; i < N; ++i) {
                result += data[i] * data[i];
            }
            return result;
        }

        T length() const {
            return std::sqrt(length_squared());
        }

        void normalize() {
            T len = length();
            if (len > 1e-6) { // Avoid division by zero
                *this = *this / len;
            }
        }

        // --- Ϊ�˷��㣬���ǿ������ .x, .y, .z, .w ������ ---
        // --- ʹ�� if constexpr (C++17) ��ȷ��ֻ��ά���㹻ʱ�ű�����Щ���� ---
        T& x() { if constexpr (N > 0) return data[0]; }
        const T& x() const { if constexpr (N > 0) return data[0]; }
        T& y() { if constexpr (N > 1) return data[1]; }
        const T& y() const { if constexpr (N > 1) return data[1]; }
        T& z() { if constexpr (N > 2) return data[2]; }
        const T& z() const { if constexpr (N > 2) return data[2]; }
        T& w() { if constexpr (N > 3) return data[3]; }
        const T& w() const { if constexpr (N > 3) return data[3]; }
    };

    // �������ͱ���
    using Vector2f = vec<2, float>;
    using Vector3f = vec<3, float>;
    using Vector4f = vec<4, float>;

    // ����������ֲ��䣬��Ҫȷ�����ܷ��� .x, .y, .z
    inline Vector3f cross(const Vector3f& a, const Vector3f& b) {
        return { a.y() * b.z() - a.z() * b.y(),
                 a.z() * b.x() - a.x() * b.z(),
                 a.x() * b.y() - a.y() * b.x() };
    }

    // ���� * ���� (��һ��д�������� a * v)
    template<size_t N, typename T>
    inline vec<N, T> operator*(T scalar, const vec<N, T>& v) {
        return v * scalar;
    }

    // ��� (Dot Product)
    template<size_t N, typename T>
    inline T dot(const vec<N, T>& a, const vec<N, T>& b) {
        T result = 0;
        for (size_t i = 0; i < N; ++i) {
            result += a[i] * b[i];
        }
        return result;
    }

    // ��һ�� (���������������޸�ԭ����)
    template<size_t N, typename T>
    inline vec<N, T> normalize(const vec<N, T>& v) {
        T len = v.length();
        if (len > 1e-6) {
            return v / len;
        }
        return v;
    }

    // ���� (Reflect)
    template<size_t N, typename T>
    inline vec<N, T> reflect(const vec<N, T>& i, const vec<N, T>& n) {
        return i - 2 * dot(i, n) * n;
    }
}
