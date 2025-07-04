// src/renderer/Clipping.h (新文件)
#pragma once
#include "IShader.h" // For Varyings
#include <vector>

namespace Morpheus::Renderer {

    // 裁剪函数：输入一个三角形的三个Varying，输出一个或多个裁剪后的三角形。
    // 输出的vector大小总是3的倍数，每3个Varying代表一个三角形。
    std::vector<Varyings> ClipTriangle(
        const Varyings& v0,
        const Varyings& v1,
        const Varyings& v2
    );

}
