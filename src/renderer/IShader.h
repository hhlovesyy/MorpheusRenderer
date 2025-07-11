// src/renderer/IShader.h (新文件)
#pragma once
#include "../math/Vector.h"
#include "../math/Matrix.h"
#include "RenderState.h"
#include <map>
#include <string>
#include <any> // C++17, 用于存储任意类型的uniforms

// 前向声明
namespace Morpheus::Renderer { struct Vertex; struct Material; class Texture; class RenderState; }
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
        Math::Vector2f uv;

        // 我们不再需要插值 world_normal，因为光照计算在切线空间进行
        // 我们需要的是将向量变换到切线空间的矩阵
        Math::Vector3f tangent_space_light_dir;
        Math::Vector3f tangent_space_view_dir;
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
        virtual Math::Vector4f FragmentShader(const Varyings& in, const RenderState& renderState) = 0;

        // 用于设置 Uniforms
        void SetUniform(const std::string& name, const std::any& value) {
            uniforms[name] = value;
        }

        // 用于获取 Uniforms (如果需要，例如在 FragmentShader 内部直接访问)
        template<typename T>
        T GetUniform(const std::string& name) const {
            try {
                return std::any_cast<T>(uniforms.at(name));
            }
            catch (const std::out_of_range& oor) {
                // Log error or throw exception
                SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Uniform '%s' not found.", name.c_str());
                // Return default value or throw
                return T{};
            }
            catch (const std::bad_any_cast& e) {
                // Log error or throw exception
                SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Uniform '%s' has wrong type: %s", name.c_str(), e.what());
                // Return default value or throw
                return T{};
            }
        }

        // --- Uniforms ---
        // 用于传递全局变量，如矩阵、相机位置、灯光等
        // 使用 std::any 提供了极大的灵活性
        std::map<std::string, std::any> uniforms;
    };
};
