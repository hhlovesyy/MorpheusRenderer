// src/renderer/Material.h (新文件)
#pragma once
#include "../math/Vector.h"

namespace Morpheus::Renderer {

    // 暂时用一个简单的结构体
    struct Material {
        Math::Vector4f color{ 1.0f, 1.0f, 1.0f, 1.0f };
        // 未来会添加:
        // std::shared_ptr<IShader> shader;
        // std::shared_ptr<Texture> albedoMap;
        // float roughness;
        // float metallic;
    };
}
