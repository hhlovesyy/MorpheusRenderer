#include "Framebuffer.h"
#include <algorithm>

namespace Morpheus::Renderer {

    // --- ����һ���������������ڽ� uint32_t ����� Vector4f ---
    // ע�⣺���������̲����� Gamma ���룬��Ϊ���ǻ��ʱ��Ҫ������ɫ
    Math::Vector4f from_color_linear(uint32_t c) {
        float r_gamma = ((c >> 16) & 0xFF) / 255.0f;
        float g_gamma = ((c >> 8) & 0xFF) / 255.0f;
        float b_gamma = (c & 0xFF) / 255.0f;
        float a = ((c >> 24) & 0xFF) / 255.0f;

        // ��������֮ǰ�� Gamma 2.2 ����
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

    // --- ���� ClearDepth ��ʵ�� ---
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

        // ��Ȳ��� (�������嶼����)
        if (z > m_depthBuffer[index]) {
            return; // ���ڵ�
        }

        if (enable_blending) {
            // --- ��͸�����壺Alpha ��� ---
            Math::Vector4f dst_color = from_color_linear(m_colorBuffer[index]);
            float src_alpha = color.w();

            Math::Vector4f final_color;
            final_color.x() = color.x() * src_alpha + dst_color.x() * (1.0f - src_alpha);
            final_color.y() = color.y() * src_alpha + dst_color.y() * (1.0f - src_alpha);
            final_color.z() = color.z() * src_alpha + dst_color.z() * (1.0f - src_alpha);
            final_color.w() = dst_color.w(); // Alpha ֵ���Լ̳б����ģ�����Ϊ1

            m_colorBuffer[index] = to_color(final_color);
            // ��͸�����岻д����ȣ�
        }
        else {
            // --- ��͸�����壺ֱ��д�� ---
            m_colorBuffer[index] = to_color(color);
        }

        // ���ݱ�־�����Ƿ�д�����
        if (write_depth) {
            m_depthBuffer[index] = z;
        }
    }
}
