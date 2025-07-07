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


    void Framebuffer::ClearColor(const Math::Vector4f& color) {
        std::fill(m_colorBuffer.begin(), m_colorBuffer.end(), to_color(color));
    }

    Framebuffer::Framebuffer(int width, int height, bool isShadowMap)
        : m_width(width), m_height(height) {
        if (isShadowMap) {
            // ֻ������Ȼ�����������ʼ����ɫ������
            m_depthBuffer.resize(width * height, std::numeric_limits<float>::max());
            // ��ʱ������ m_depthTexture������ Renderer ����
            // m_depthTexture = nullptr; // �� Texture::CreateDepthTexture(...)
        }
        else {
            // ��׼ Framebuffer������ɫ����Ȼ�����
            m_colorBuffer.resize(width * height);
            m_depthBuffer.resize(width * height, std::numeric_limits<float>::max());
        }
    }

    // --- ���� SetDepth ��ʵ�� ---
    void Framebuffer::SetDepth(int x, int y, float depth) {
        if (x < 0 || x >= m_width || y < 0 || y >= m_height) {
            return;
        }
        int index = (m_height - 1 - y) * m_width + x; // ȷ�� Y �ᷭתһ����
        // �� Shadow Pass �У�����ֱ��д����ȣ������в��� (��Ϊ�ǵ�һ�� Pass)
        m_depthBuffer[index] = depth;
    }

    // --- �޸� SetPixel ���� ---
    void Framebuffer::SetPixel(int x, int y, float depth, const Math::Vector4f& color, const RenderState& state) {
        if (x < 0 || x >= m_width || y < 0 || y >= m_height) {
            return;
        }

        int index = (m_height - 1 - y) * m_width + x; // Y�ᷭת����

        // --- ��Ȳ��� ---
        // ���� RenderState ����ʱ������Ȳ���
        bool perform_depth_test = state.IsFlagEnabled(RenderStateFlags::DepthTestEnable);
        if (perform_depth_test) {
            // ���ֵԽСԽ��
            if (depth < m_depthBuffer[index]) {
                // ͨ����Ȳ��ԣ���������
            }
            else {
                return; // ���ڵ�
            }
        }
        // �����������Ȳ��ԣ���ֱ��ͨ��

        // --- ��ɫд��� Alpha ��� ---
        bool blending_enabled = state.IsFlagEnabled(RenderStateFlags::BlendEnable);
        if (blending_enabled) {
            // --- ��͸�����壺Alpha ��� ---
            Math::Vector4f dst_color = from_color_linear(m_colorBuffer[index]);
            float src_alpha = color.w();

            Math::Vector4f final_color;
            final_color.x() = color.x() * src_alpha + dst_color.x() * (1.0f - src_alpha);
            final_color.y() = color.y() * src_alpha + dst_color.y() * (1.0f - src_alpha);
            final_color.z() = color.z() * src_alpha + dst_color.z() * (1.0f - src_alpha);
            // Alpha ���: alpha_dst = alpha_src + alpha_dst * (1 - alpha_src)
            final_color.w() = src_alpha + dst_color.w() * (1.0f - src_alpha);

            m_colorBuffer[index] = to_color(final_color);
            // ��͸������ͨ����д����ȣ��������ض�����
            // ������� state.IsFlagEnabled(RenderStateFlags::DepthWriteEnable) ������
            // ��� DepthWriteEnable �� true�����������͸��д����ȣ���д��
            // �ڴ��������£���͸��ʱ��д�����
            if (state.IsFlagEnabled(RenderStateFlags::DepthWriteEnable)) {
                // �����Ƿ������͸��д����ȣ�ͨ���� false
                // if (!blending_enabled) { // ֻ�в�͸��ʱ��д�����
                m_depthBuffer[index] = depth;
                // }
            }

        }
        else {
            // --- ��͸�����壺ֱ��д�� ---
            m_colorBuffer[index] = to_color(color);
            // ��� RenderState �������д�룬��д�����
            if (state.IsFlagEnabled(RenderStateFlags::DepthWriteEnable)) {
                m_depthBuffer[index] = depth;
            }
        }
    }

    // --- ClearDepth ��ʵ�� ---
    void Framebuffer::ClearDepth(float depth) {
        std::fill(m_depthBuffer.begin(), m_depthBuffer.end(), depth);
    }


}
