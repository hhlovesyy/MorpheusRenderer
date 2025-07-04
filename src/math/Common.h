// src/math/Common.h (���ļ�)
#pragma once

namespace Morpheus::Math {
    // ģ�廯�����Բ�ֵ����
    template<typename T>
    T Lerp(const T& a, const T& b, float t) {
        return a * (1.0f - t) + b * t;
    }
}
