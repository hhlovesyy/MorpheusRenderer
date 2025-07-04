#pragma once
#include <vector>
#include <cstdint>
#include "../math/Vector.h" // ע�����·��

namespace Morpheus::Renderer {
    class Framebuffer {
    public:
        Framebuffer(int width, int height);
        void ClearColor(const Math::Vector4f& color);
        void ClearDepth(float depth = 1.0f); // <--- �����������Ȼ���
        // --- �޸� SetPixel������������Ȳ����߼� ---
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
