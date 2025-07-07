#pragma once
#include "../math/Vector.h"
#include <string>
#include <vector>
#include <memory>

namespace Morpheus::Renderer {
    enum class TextureFormat {
        RGB_UCHAR, // 8-bit RGB
        RGBA_UCHAR, // 8-bit RGBA
        // ... ������ʽ ...
        DEPTH_FLOAT // ����Ϊ�� Shadow Map ���
    };

    class Texture {
    public:
        // ���ļ���������
        static std::shared_ptr<Texture> Load(const std::string& filepath);

        // �����������
        Math::Vector4f Sample(float u, float v) const;

        int GetWidth() const { return m_width; }
        int GetHeight() const { return m_height; }
        Texture(unsigned char* data, int width, int height, int channels);

        // TextureFormat GetFormat() const { return m_format; } // ����ж���
        // --- sRGB -> Linear ת�� ---
        // �������Ӧ�÷��� Texture ���ڲ�����Ϊ����������غͽ��͵�һ����
        static Math::Vector4f srgb_to_linear(const Math::Vector4f& srgb_color);

    private:
        int m_width;
        int m_height;
        int m_channels;
        std::vector<unsigned char> m_data;
    };
}
