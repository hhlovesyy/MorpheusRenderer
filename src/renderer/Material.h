// src/renderer/Material.h (���ļ�)
#pragma once
#include "../math/Vector.h"

namespace Morpheus::Renderer {

    // ��ʱ��һ���򵥵Ľṹ��
    struct Material {
        Math::Vector4f color{ 1.0f, 1.0f, 1.0f, 1.0f };
        // δ�������:
        // std::shared_ptr<IShader> shader;
        // std::shared_ptr<Texture> albedoMap;
        // float roughness;
        // float metallic;
    };
}
