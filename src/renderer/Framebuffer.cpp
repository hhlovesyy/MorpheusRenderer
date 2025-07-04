#include "Framebuffer.h"
#include <algorithm>

namespace Morpheus::Renderer {

    // --- 新增一个辅助函数，用于将 uint32_t 解码回 Vector4f ---
    // 注意：这个解码过程不包含 Gamma 解码，因为我们混合时需要线性颜色
    Math::Vector4f from_color_linear(uint32_t c) {
        float r_gamma = ((c >> 16) & 0xFF) / 255.0f;
        float g_gamma = ((c >> 8) & 0xFF) / 255.0f;
        float b_gamma = (c & 0xFF) / 255.0f;
        float a = ((c >> 24) & 0xFF) / 255.0f;

        // 假设我们之前用 Gamma 2.2 编码
        return {
            std::pow(r_gamma, 2.2f),
            std::pow(g_gamma, 2.2f),
            std::pow(b_gamma, 2.2f),
            a
        };
    }

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

    void Framebuffer::SetPixel(int x, int y, float z, const Math::Vector4f& color, bool write_depth, bool enable_blending) {
        if (x < 0 || x >= m_width || y < 0 || y >= m_height) {
            return;
        }

        int index = (m_height - 1 - y) * m_width + x;

        // 深度测试 (所有物体都进行)
        if (z > m_depthBuffer[index]) {
            return; // 被遮挡
        }

        if (enable_blending) {
            // --- 半透明物体：Alpha 混合 ---
            Math::Vector4f dst_color = from_color_linear(m_colorBuffer[index]);
            float src_alpha = color.w();

            Math::Vector4f final_color;
            final_color.x() = color.x() * src_alpha + dst_color.x() * (1.0f - src_alpha);
            final_color.y() = color.y() * src_alpha + dst_color.y() * (1.0f - src_alpha);
            final_color.z() = color.z() * src_alpha + dst_color.z() * (1.0f - src_alpha);
            final_color.w() = dst_color.w(); // Alpha 值可以继承背景的，或设为1

            m_colorBuffer[index] = to_color(final_color);
            // 半透明物体不写入深度！
        }
        else {
            // --- 不透明物体：直接写入 ---
            m_colorBuffer[index] = to_color(color);
        }

        // 根据标志决定是否写入深度
        if (write_depth) {
            m_depthBuffer[index] = z;
        }
    }
}
