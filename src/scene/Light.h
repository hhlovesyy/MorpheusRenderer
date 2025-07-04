// src/scene/Light.h (���ļ�)
#pragma once
#include "../math/Vector.h"

namespace Morpheus::Scene {

    // ������ֻ֧�ַ����
    struct DirectionalLight {
        Math::Vector3f direction;
        Math::Vector3f color;
        float intensity;
    };

    // δ��������չ�� PointLight, SpotLight ��
}
