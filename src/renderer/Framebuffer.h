#pragma once
#include <vector>
#include <cstdint>
#include "../math/Vector.h" // 注意相对路径

namespace Morpheus::Renderer {
    class Framebuffer {
    public:
        Framebuffer(int width, int height);
        void ClearColor(const Math::Vector4f& color);
        void ClearDepth(float depth = 1.0f); // <--- 新增：清空深度缓冲
        // --- 修改 SetPixel，让它包含深度测试逻辑 ---
        void SetPixel(int x, int y, float z, const Math::Vector4f& color, bool write_depth, bool enable_blending);
        int GetWidth() const { return m_width; }
        int GetHeight() const { return m_height; }
        const uint32_t* GetPixelData() const { return m_colorBuffer.data(); }
    private:
        int m_width;
        int m_height;
        std::vector<uint32_t> m_colorBuffer;
        std::vector<float> m_depthBuffer; 
    };
}
