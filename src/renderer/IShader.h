// src/renderer/IShader.h (新文件)
#pragma once
#include "../math/Vector.h"
#include "../math/Matrix.h"
#include <map>
#include <string>
#include <any> // C++17, 用于存储任意类型的uniforms

// 前向声明
namespace Morpheus::Renderer { struct Vertex; struct Material; class Texture; }
namespace Morpheus::Scene { class Light; }


namespace Morpheus::Renderer {
    struct Vertex;
    // --- 定义顶点着色器的输出，片元着色器的输入 ---
    // 这个结构体将携带所有需要从VS传递到FS并进行插值的属性
    struct Varyings {
        // 每个顶点都必须有裁剪空间坐标
        Math::Vector4f position_clip;
        Math::Vector3f world_pos;
        // --- 为演示和未来光照准备的属性 ---
        Math::Vector3f color; // 我们用这个来传递和插值颜色

        // 自定义数据，例如：
        // Math::Vector3f world_pos;
        Math::Vector3f world_normal;
        // Math::Vector2f uv;
    };

    // --- Shader 接口 ---
    class IShader {
    public:
        virtual ~IShader() = default;

        // --- 顶点着色器 ---
        // 输入: 单个顶点属性
        // 输出: 经过插值的Varyings结构体
        virtual Varyings VertexShader(const Vertex& in) = 0;

        // --- 片元着色器 ---
        // 输入: 经过透视校正插值后的Varyings
        // 输出: 最终的像素颜色 (RGBA)
        virtual Math::Vector4f FragmentShader(const Varyings& in) = 0;

        // --- Uniforms ---
        // 用于传递全局变量，如矩阵、相机位置、灯光等
        // 使用 std::any 提供了极大的灵活性
        std::map<std::string, std::any> uniforms;
    };
};
