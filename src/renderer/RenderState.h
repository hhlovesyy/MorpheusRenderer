// src/renderer/RenderState.h (或者包含在 src/renderer/Renderer.h 中)
#pragma once

#include "../math/Vector.h"
#include "../math/Matrix.h"
#include <cstdint> // For uint32_t

// Forward declarations (如果 Framebuffer 和 Texture 是在其他头文件中定义的)
namespace Morpheus::Renderer {
    class Framebuffer;
    class Texture;
}

namespace Morpheus::Renderer {

    // --- 渲染状态标志 ---
    // 使用 enum class 提供类型安全和更好的可读性
    enum class RenderStateFlags {
        None = 0,
        DepthWriteEnable = 1 << 0,  // 是否写入深度缓冲
        DepthTestEnable = 1 << 1,  // 是否进行深度测试
        CullFaceEnable = 1 << 2,  // 是否启用背面剔除
        BlendEnable = 1 << 3,  // 是否启用 Alpha 混合
        // 可以添加更多标志，例如：StencilTestEnable, WireframeEnable 等
    };

    // --- RenderState 结构体 ---
    // 这个结构体会作为参数传递给各个 RenderPass 和 Shader
    struct RenderState {
        // --- 渲染目标 ---
        Framebuffer* targetFramebuffer = nullptr; // 当前渲染操作的目标 Framebuffer

        // --- Shadow Map 相关信息 ---
        // 指向 Shadow Map 的深度纹理
        // 注意：在初始阶段，我们可以直接传递这个纹理指针给 Shader
        // 稍后可能会有更完善的纹理单元管理器
        const Texture* shadowMapTexture = nullptr;
        // 光源的 View-Projection 矩阵，用于将世界坐标转换到光源的裁剪空间
        Math::Matrix4f lightViewProjectionMatrix;

        // --- 渲染模式标志 ---
        // 组合了上面的 RenderStateFlags
        // 使用整数类型来存储标志位，方便进行位运算
        uint32_t flags = static_cast<uint32_t>(RenderStateFlags::None);

        // --- 辅助方法：用于设置和检查标志位 ---

        // 添加一个或多个标志
        void AddFlags(RenderStateFlags newFlags) {
            flags |= static_cast<uint32_t>(newFlags);
        }

        // 移除一个标志
        void RemoveFlags(RenderStateFlags flagsToRemove) {
            flags &= ~static_cast<uint32_t>(flagsToRemove);
        }

        // 检查是否启用了某个标志
        bool IsFlagEnabled(RenderStateFlags flag) const {
            return (flags & static_cast<uint32_t>(flag)) != 0;
        }

        // --- 构造函数 (可选，但方便初始化) ---
        RenderState() = default; // 使用默认构造函数，状态可以稍后设置
    };

} // namespace Morpheus::Renderer
