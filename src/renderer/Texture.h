#pragma once
#include "../math/Vector.h"
#include <string>
#include <vector>
#include <memory>

namespace Morpheus::Renderer {

    class Texture {
    public:
        // 从文件加载纹理
        static std::shared_ptr<Texture> Load(const std::string& filepath);

        // 纹理采样函数
        Math::Vector4f Sample(float u, float v) const;

        int GetWidth() const { return m_width; }
        int GetHeight() const { return m_height; }
        Texture(unsigned char* data, int width, int height, int channels);

    private:
        int m_width;
        int m_height;
        int m_channels;
        std::vector<unsigned char> m_data;
    };
}
