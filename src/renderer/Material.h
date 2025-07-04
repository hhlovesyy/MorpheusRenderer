// src/renderer/Material.h (新文件)
#pragma once
#include "../math/Vector.h"
#include "../renderer/IShader.h"
#include "Texture.h"

namespace Morpheus::Renderer {

    // 暂时用一个简单的结构体
    struct Material {
        Math::Vector4f albedo_factor{ 1.0f, 1.0f, 1.0f, 1.0f }; // 基础颜色
		std::shared_ptr<Texture> albedo_texture; // 颜色贴图
        float specular_shininess = 32.0f; // 高光指数 (shininess)
        // 未来会添加:
        std::shared_ptr<IShader> shader;
        // std::shared_ptr<Texture> albedoMap;
        // float roughness;
        // float metallic;
    };
}
