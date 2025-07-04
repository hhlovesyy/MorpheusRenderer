// src/renderer/Clipping.h (���ļ�)
#pragma once
#include "IShader.h" // For Varyings
#include <vector>

namespace Morpheus::Renderer {

    // �ü�����������һ�������ε�����Varying�����һ�������ü���������Ρ�
    // �����vector��С����3�ı�����ÿ3��Varying����һ�������Ρ�
    std::vector<Varyings> ClipTriangle(
        const Varyings& v0,
        const Varyings& v1,
        const Varyings& v2
    );

}
