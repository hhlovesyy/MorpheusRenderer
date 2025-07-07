#pragma once
#include "../math/Vector.h"
#include <string>
#include <vector>
#include <memory>

namespace Morpheus::Renderer {
    enum class TextureFormat {
        RGB_UCHAR, // 8-bit RGB
        RGBA_UCHAR, // 8-bit RGBA
        // ... 其他格式 ...
        DEPTH_FLOAT // 假设为了 Shadow Map 添加
    };

    class Texture {
    public:
        // 从文件加载纹理
        static std::shared_ptr<Texture> Load(const std::string& filepath);

        // 纹理采样函数
        Math::Vector4f Sample(float u, float v) const;

        int GetWidth() const { return m_width; }
        int GetHeight() const { return m_height; }
        Texture(unsigned char* data, int width, int height, int channels);

        // TextureFormat GetFormat() const { return m_format; } // 如果有定义
        // --- sRGB -> Linear 转换 ---
        // 这个函数应该放在 Texture 类内部，因为它是纹理加载和解释的一部分
        static Math::Vector4f srgb_to_linear(const Math::Vector4f& srgb_color);

    private:
        int m_width;
        int m_height;
        int m_channels;
        std::vector<unsigned char> m_data;
    };
}
