// src/renderer/Material.h (���ļ�)
#pragma once
#include "../math/Vector.h"
#include "../renderer/IShader.h"
#include "Texture.h"

namespace Morpheus::Renderer {

    class IShader;
    // --- ������������Ⱦ���е����� ---
    enum class RenderQueue {
        Opaque = 0,      // ��͸������ (ʹ������Unity�Ķ���ֵ)
        Skybox,      // ��պ�
        Transparent, // ��͸������
        Count          // ��������
    };

    // ��ʱ��һ���򵥵Ľṹ��
    struct Material {
		RenderQueue render_queue = RenderQueue::Opaque; 

        Math::Vector4f albedo_factor{ 1.0f, 1.0f, 1.0f, 1.0f }; // ������ɫ
		std::shared_ptr<Texture> albedo_texture; // ��ɫ��ͼ
        std::shared_ptr<Texture> normal_texture; // <--- ����������ͼ
        float specular_shininess = 32.0f; // �߹�ָ�� (shininess)
        // δ�������:
        std::shared_ptr<IShader> shader;
		float alpha_factor = 1.0f; 
        // std::shared_ptr<Texture> albedoMap;
        // float roughness;
        // float metallic;
    };
}
