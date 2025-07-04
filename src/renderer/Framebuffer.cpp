#include "Framebuffer.h"
#include <algorithm>

namespace Morpheus::Renderer {

    // Helper to convert float color [0,1] to uint32_t 0xAARRGGBB
    uint32_t to_color(const Math::Vector4f& c) {
        uint8_t r = static_cast<uint8_t>(std::clamp(c.x(), 0.0f, 1.0f) * 255.0f);
        uint8_t g = static_cast<uint8_t>(std::clamp(c.y(), 0.0f, 1.0f) * 255.0f);
        uint8_t b = static_cast<uint8_t>(std::clamp(c.z(), 0.0f, 1.0f) * 255.0f);
        uint8_t a = static_cast<uint8_t>(std::clamp(c.w(), 0.0f, 1.0f) * 255.0f);
        return (a << 24) | (r << 16) | (g << 8) | b;
    }

    Framebuffer::Framebuffer(int width, int height)
        : m_width(width), m_height(height) {
        m_colorBuffer.resize(width * height);
        m_depthBuffer.resize(width * height);
    }

    // --- 新增 ClearDepth 的实现 ---
    void Framebuffer::ClearDepth(float depth) {
        std::fill(m_depthBuffer.begin(), m_depthBuffer.end(), depth);
    }

    void Framebuffer::ClearColor(const Math::Vector4f& color) {
        std::fill(m_colorBuffer.begin(), m_colorBuffer.end(), to_color(color));
    }

    void Framebuffer::SetPixel(int x, int y, float z, const Math::Vector4f& color) {
        if (x >= 0 && x < m_width && y >= 0 && y < m_height) {
            int index = (m_height - 1 - y) * m_width + x;

            // 深度测试：如果新的z值比缓冲区里的小，就更新
            if (z < m_depthBuffer[index]) {
                m_depthBuffer[index] = z; // 更新深度值
                m_colorBuffer[index] = to_color(color); // 更新颜色值
            }
        }
    }
}
