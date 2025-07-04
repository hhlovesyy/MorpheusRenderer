// src/renderer/Material.h (���ļ�)
#pragma once
#include "../math/Vector.h"
#include "../renderer/IShader.h"
#include "Texture.h"

namespace Morpheus::Renderer {

    // ��ʱ��һ���򵥵Ľṹ��
    struct Material {
        Math::Vector4f albedo_factor{ 1.0f, 1.0f, 1.0f, 1.0f }; // ������ɫ
		std::shared_ptr<Texture> albedo_texture; // ��ɫ��ͼ
        float specular_shininess = 32.0f; // �߹�ָ�� (shininess)
        // δ�������:
        std::shared_ptr<IShader> shader;
        // std::shared_ptr<Texture> albedoMap;
        // float roughness;
        // float metallic;
    };
}
