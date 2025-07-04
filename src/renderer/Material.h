// src/renderer/Material.h (新文件)
#pragma once
#include "../math/Vector.h"
#include "../renderer/IShader.h"
#include "Texture.h"

namespace Morpheus::Renderer {

    class IShader;
    // --- 新增：定义渲染队列的类型 ---
    enum class RenderQueue {
        Opaque = 0,      // 不透明物体 (使用类似Unity的队列值)
        Skybox,      // 天空盒
        Transparent, // 半透明物体
        Count          // 队列总数
    };

    // 暂时用一个简单的结构体
    struct Material {
		RenderQueue render_queue = RenderQueue::Opaque; 

        Math::Vector4f albedo_factor{ 1.0f, 1.0f, 1.0f, 1.0f }; // 基础颜色
		std::shared_ptr<Texture> albedo_texture; // 颜色贴图
        std::shared_ptr<Texture> normal_texture; // <--- 新增法线贴图
        float specular_shininess = 32.0f; // 高光指数 (shininess)
        // 未来会添加:
        std::shared_ptr<IShader> shader;
		float alpha_factor = 1.0f; 
        // std::shared_ptr<Texture> albedoMap;
        // float roughness;
        // float metallic;
    };
}
