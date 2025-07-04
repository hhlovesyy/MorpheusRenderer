// src/renderer/IShader.h (���ļ�)
#pragma once
#include "../math/Vector.h"
#include "../math/Matrix.h"
#include <map>
#include <string>
#include <any> // C++17, ���ڴ洢�������͵�uniforms

// ǰ������
namespace Morpheus::Renderer { struct Vertex; struct Material; class Texture; }
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
        // Math::Vector2f uv;
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
        virtual Math::Vector4f FragmentShader(const Varyings& in) = 0;

        // --- Uniforms ---
        // ���ڴ���ȫ�ֱ�������������λ�á��ƹ��
        // ʹ�� std::any �ṩ�˼���������
        std::map<std::string, std::any> uniforms;
    };
};
