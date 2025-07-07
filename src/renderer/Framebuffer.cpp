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


    void Framebuffer::ClearColor(const Math::Vector4f& color) {
        std::fill(m_colorBuffer.begin(), m_colorBuffer.end(), to_color(color));
    }

    Framebuffer::Framebuffer(int width, int height, bool isShadowMap)
        : m_width(width), m_height(height) {
        if (isShadowMap) {
            // 只分配深度缓冲区，不初始化颜色缓冲区
            m_depthBuffer.resize(width * height, std::numeric_limits<float>::max());
            // 暂时不创建 m_depthTexture，留待 Renderer 管理
            // m_depthTexture = nullptr; // 或 Texture::CreateDepthTexture(...)
        }
        else {
            // 标准 Framebuffer，有颜色和深度缓冲区
            m_colorBuffer.resize(width * height);
            m_depthBuffer.resize(width * height, std::numeric_limits<float>::max());
        }
    }

    // --- 新增 SetDepth 的实现 ---
    void Framebuffer::SetDepth(int x, int y, float depth) {
        if (x < 0 || x >= m_width || y < 0 || y >= m_height) {
            return;
        }
        int index = (m_height - 1 - y) * m_width + x; // 确认 Y 轴翻转一致性
        // 在 Shadow Pass 中，我们直接写入深度，不进行测试 (因为是第一个 Pass)
        m_depthBuffer[index] = depth;
    }

    // --- 修改 SetPixel 方法 ---
    void Framebuffer::SetPixel(int x, int y, float depth, const Math::Vector4f& color, const RenderState& state) {
        if (x < 0 || x >= m_width || y < 0 || y >= m_height) {
            return;
        }

        int index = (m_height - 1 - y) * m_width + x; // Y轴翻转索引

        // --- 深度测试 ---
        // 仅当 RenderState 允许时进行深度测试
        bool perform_depth_test = state.IsFlagEnabled(RenderStateFlags::DepthTestEnable);
        if (perform_depth_test) {
            // 深度值越小越近
            if (depth < m_depthBuffer[index]) {
                // 通过深度测试，继续处理
            }
            else {
                return; // 被遮挡
            }
        }
        // 如果不进行深度测试，则直接通过

        // --- 颜色写入和 Alpha 混合 ---
        bool blending_enabled = state.IsFlagEnabled(RenderStateFlags::BlendEnable);
        if (blending_enabled) {
            // --- 半透明物体：Alpha 混合 ---
            Math::Vector4f dst_color = from_color_linear(m_colorBuffer[index]);
            float src_alpha = color.w();

            Math::Vector4f final_color;
            final_color.x() = color.x() * src_alpha + dst_color.x() * (1.0f - src_alpha);
            final_color.y() = color.y() * src_alpha + dst_color.y() * (1.0f - src_alpha);
            final_color.z() = color.z() * src_alpha + dst_color.z() * (1.0f - src_alpha);
            // Alpha 混合: alpha_dst = alpha_src + alpha_dst * (1 - alpha_src)
            final_color.w() = src_alpha + dst_color.w() * (1.0f - src_alpha);

            m_colorBuffer[index] = to_color(final_color);
            // 半透明物体通常不写入深度，除非有特定需求
            // 这里根据 state.IsFlagEnabled(RenderStateFlags::DepthWriteEnable) 来决定
            // 如果 DepthWriteEnable 是 true，并且允许半透明写入深度，则写入
            // 在大多数情况下，半透明时不写入深度
            if (state.IsFlagEnabled(RenderStateFlags::DepthWriteEnable)) {
                // 考虑是否允许半透明写入深度，通常是 false
                // if (!blending_enabled) { // 只有不透明时才写入深度
                m_depthBuffer[index] = depth;
                // }
            }

        }
        else {
            // --- 不透明物体：直接写入 ---
            m_colorBuffer[index] = to_color(color);
            // 如果 RenderState 允许深度写入，则写入深度
            if (state.IsFlagEnabled(RenderStateFlags::DepthWriteEnable)) {
                m_depthBuffer[index] = depth;
            }
        }
    }

    // --- ClearDepth 的实现 ---
    void Framebuffer::ClearDepth(float depth) {
        std::fill(m_depthBuffer.begin(), m_depthBuffer.end(), depth);
    }


}
