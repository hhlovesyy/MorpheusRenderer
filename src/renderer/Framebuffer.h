#pragma once
#include <vector>
#include <cstdint>
#include "../math/Vector.h" // 注意相对路径
#include "RenderState.h"

namespace Morpheus::Renderer {
    class Framebuffer {
    public:
        Framebuffer(int width, int height);

        // --- 新增：为 Shadow Map 创建一个只包含深度缓冲的 Framebuffer ---
        // 这个构造函数将分配一个深度缓冲区，但不会关联颜色缓冲区
        // 并会创建一个对应的深度纹理对象
        Framebuffer(int width, int height, bool isShadowMap);

        void ClearColor(const Math::Vector4f& color);
        void ClearDepth(float depth = 1.0f); // <--- 新增：清空深度缓冲
        // --- 修改 SetPixel，让它包含深度测试逻辑 ---
        void SetPixel(int x, int y, float z, const Math::Vector4f& color, const RenderState& state);
		void SetDepth(int x, int y, float depth); //新增SetDepth方法，用于shadow pass
        
        int GetWidth() const { return m_width; }
        int GetHeight() const { return m_height; }
        const uint32_t* GetPixelData() const { return m_colorBuffer.data(); }
        // 获取深度纹理的指针 (用于 Shadow Pass 的创建和后续渲染)
        const Texture* GetDepthTexture() const { return m_depthTexture.get(); }
        // 获取颜色缓冲区数据 (用于最终输出)
        const std::vector<uint32_t>& GetColorBuffer() const { return m_colorBuffer; }
    private:
        int m_width;
        int m_height;
        std::vector<uint32_t> m_colorBuffer;
        std::vector<float> m_depthBuffer; 

        // --- Shadow Map Framebuffer 特有 ---
       // 用于 Shadow Map 的深度纹理
        std::shared_ptr<Texture> m_depthTexture;
    };
}
