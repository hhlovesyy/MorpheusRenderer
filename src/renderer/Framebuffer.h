#pragma once
#include <vector>
#include <cstdint>
#include "../math/Vector.h" // ע�����·��
#include "RenderState.h"

namespace Morpheus::Renderer {
    class Framebuffer {
    public:
        Framebuffer(int width, int height);

        // --- ������Ϊ Shadow Map ����һ��ֻ������Ȼ���� Framebuffer ---
        // ������캯��������һ����Ȼ������������������ɫ������
        // ���ᴴ��һ����Ӧ������������
        Framebuffer(int width, int height, bool isShadowMap);

        void ClearColor(const Math::Vector4f& color);
        void ClearDepth(float depth = 1.0f); // <--- �����������Ȼ���
        // --- �޸� SetPixel������������Ȳ����߼� ---
        void SetPixel(int x, int y, float z, const Math::Vector4f& color, const RenderState& state);
		void SetDepth(int x, int y, float depth); //����SetDepth����������shadow pass
        
        int GetWidth() const { return m_width; }
        int GetHeight() const { return m_height; }
        const uint32_t* GetPixelData() const { return m_colorBuffer.data(); }
        // ��ȡ��������ָ�� (���� Shadow Pass �Ĵ����ͺ�����Ⱦ)
        const Texture* GetDepthTexture() const { return m_depthTexture.get(); }
        // ��ȡ��ɫ���������� (�����������)
        const std::vector<uint32_t>& GetColorBuffer() const { return m_colorBuffer; }
    private:
        int m_width;
        int m_height;
        std::vector<uint32_t> m_colorBuffer;
        std::vector<float> m_depthBuffer; 

        // --- Shadow Map Framebuffer ���� ---
       // ���� Shadow Map ���������
        std::shared_ptr<Texture> m_depthTexture;
    };
}
