// src/math/Common.h (新文件)
#pragma once

namespace Morpheus::Math {
    // 模板化的线性插值函数
    template<typename T>
    T Lerp(const T& a, const T& b, float t) {
        return a * (1.0f - t) + b * t;
    }
}
