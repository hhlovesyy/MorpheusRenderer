#include "Texture.h"
#include <iostream>
#include <SDL.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <algorithm>

namespace Morpheus::Renderer {

    Texture::Texture(unsigned char* data, int width, int height, int channels)
        : m_width(width), m_height(height), m_channels(channels) {
        size_t size = width * height * channels;
        m_data.assign(data, data + size);
        stbi_image_free(data); // stb_image ���غ���Ҫ�ͷ��ڴ�
    }

    // --- ʵ�� sRGB -> Linear ת�� ---
    // ���ʵ�ֻ��� ITU-R BT.709 ��׼
    Math::Vector4f Texture::srgb_to_linear(const Math::Vector4f& srgb_color) {
        // Alpha ͨ������Ҫת��
        Math::Vector4f linear_color = srgb_color;

        for (int i = 0; i < 3; ++i) { // R, G, B
            float c = srgb_color[i];
            if (c <= 0.04045f) {
                linear_color[i] = c / 12.92f;
            }
            else {
                linear_color[i] = std::pow((c + 0.055f) / 1.055f, 2.4f);
            }
        }
        return linear_color;
    }

    std::shared_ptr<Texture> Texture::Load(const std::string& filepath) {
        int width, height, channels;
        // ǿ�Ƽ���Ϊ4ͨ�� (RGBA)�����㴦��
        unsigned char* data = stbi_load(filepath.c_str(), &width, &height, &channels, 4);
        if (!data) {
            SDL_Log("Failed to load texture: %s", filepath.c_str());
            return nullptr;
        }
        SDL_Log("Loaded texture: %s (%dx%d)", filepath.c_str(), width, height);
        return std::make_shared<Texture>(data, width, height, 4);
    }

    // ����ڲ��� (Nearest Neighbor) - ��򵥵�ʵ��
    Math::Vector4f Texture::Sample(float u, float v) const {
        if (m_data.empty()) {
            return { 1.0f, 0.0f, 1.0f, 1.0f }; // ��������ɫ�Ա�ʾ����
        }

        // �� UV ���������� [0, 1] ��Χ (Repeat wrapping)
        u = u - floor(u);
        v = 1.0f - (v - floor(v)); // ��������ͨ��Y���Ƿ���

        int x = static_cast<int>(u * (m_width - 1));
        int y = static_cast<int>(v * (m_height - 1));

        x = std::clamp(x, 0, m_width - 1);
        y = std::clamp(y, 0, m_height - 1);

        int index = (y * m_width + x) * m_channels;

        // �� [0, 255] ����ɫֵת��Ϊ [0, 1] �ĸ�����
        float r = m_data[index + 0] / 255.0f;
        float g = m_data[index + 1] / 255.0f;
        float b = m_data[index + 2] / 255.0f;
        float a = (m_channels == 4) ? m_data[index + 3] / 255.0f : 1.0f;

        return { r, g, b, a }; //todo:Linear2srgb���߼���Ҫ���ж����жϣ���Щ��ͼ��Ҫת����Щ����ת
    }
}
