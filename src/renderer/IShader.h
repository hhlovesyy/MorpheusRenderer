// src/renderer/IShader.h (���ļ�)
#pragma once
#include "../math/Vector.h"
#include "../math/Matrix.h"
#include "RenderState.h"
#include <map>
#include <string>
#include <any> // C++17, ���ڴ洢�������͵�uniforms

// ǰ������
namespace Morpheus::Renderer { struct Vertex; struct Material; class Texture; class RenderState; }
namespace Morpheus::Scene { class Light; }


namespace Morpheus::Renderer {
    struct Vertex;
    // --- ���嶥����ɫ���������ƬԪ��ɫ�������� ---
    // ����ṹ�彫Я��������Ҫ��VS���ݵ�FS�����в�ֵ������
    struct Varyings {
        // ÿ�����㶼�����вü��ռ�����
        Math::Vector4f position_clip;
        Math::Vector3f world_pos;
        // --- Ϊ��ʾ��δ������׼�������� ---
        Math::Vector3f color; // ��������������ݺͲ�ֵ��ɫ

        // �Զ������ݣ����磺
        // Math::Vector3f world_pos;
        Math::Vector3f world_normal;
        Math::Vector2f uv;

        // ���ǲ�����Ҫ��ֵ world_normal����Ϊ���ռ��������߿ռ����
        // ������Ҫ���ǽ������任�����߿ռ�ľ���
        Math::Vector3f tangent_space_light_dir;
        Math::Vector3f tangent_space_view_dir;
    };

    // --- Shader �ӿ� ---
    class IShader {
    public:
        virtual ~IShader() = default;

        // --- ������ɫ�� ---
        // ����: ������������
        // ���: ������ֵ��Varyings�ṹ��
        virtual Varyings VertexShader(const Vertex& in) = 0;

        // --- ƬԪ��ɫ�� ---
        // ����: ����͸��У����ֵ���Varyings
        // ���: ���յ�������ɫ (RGBA)
        virtual Math::Vector4f FragmentShader(const Varyings& in, const RenderState& renderState) = 0;

        // �������� Uniforms
        void SetUniform(const std::string& name, const std::any& value) {
            uniforms[name] = value;
        }

        // ���ڻ�ȡ Uniforms (�����Ҫ�������� FragmentShader �ڲ�ֱ�ӷ���)
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
        // ���ڴ���ȫ�ֱ�������������λ�á��ƹ��
        // ʹ�� std::any �ṩ�˼���������
        std::map<std::string, std::any> uniforms;
    };
};
