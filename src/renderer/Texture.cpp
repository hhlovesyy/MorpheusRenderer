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
        stbi_image_free(data); // stb_image 加载后需要释放内存
    }

    // --- 实现 sRGB -> Linear 转换 ---
    // 这个实现基于 ITU-R BT.709 标准
    Math::Vector4f Texture::srgb_to_linear(const Math::Vector4f& srgb_color) {
        // Alpha 通道不需要转换
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
        // 强制加载为4通道 (RGBA)，方便处理
        unsigned char* data = stbi_load(filepath.c_str(), &width, &height, &channels, 4);
        if (!data) {
            SDL_Log("Failed to load texture: %s", filepath.c_str());
            return nullptr;
        }
        SDL_Log("Loaded texture: %s (%dx%d)", filepath.c_str(), width, height);
        return std::make_shared<Texture>(data, width, height, 4);
    }

    // 最近邻采样 (Nearest Neighbor) - 最简单的实现
    Math::Vector4f Texture::Sample(float u, float v) const {
        if (m_data.empty()) {
            return { 1.0f, 0.0f, 1.0f, 1.0f }; // 返回亮粉色以表示错误
        }

        // 将 UV 坐标限制在 [0, 1] 范围 (Repeat wrapping)
        u = u - floor(u);
        v = 1.0f - (v - floor(v)); // 纹理坐标通常Y轴是反的

        int x = static_cast<int>(u * (m_width - 1));
        int y = static_cast<int>(v * (m_height - 1));

        x = std::clamp(x, 0, m_width - 1);
        y = std::clamp(y, 0, m_height - 1);

        int index = (y * m_width + x) * m_channels;

        // 将 [0, 255] 的颜色值转换为 [0, 1] 的浮点数
        float r = m_data[index + 0] / 255.0f;
        float g = m_data[index + 1] / 255.0f;
        float b = m_data[index + 2] / 255.0f;
        float a = (m_channels == 4) ? m_data[index + 3] / 255.0f : 1.0f;

        return { r, g, b, a }; //todo:Linear2srgb的逻辑需要进行额外判断，有些贴图需要转，有些则不用转
    }
}
