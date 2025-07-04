// src/scene/Light.h (新文件)
#pragma once
#include "../math/Vector.h"

namespace Morpheus::Scene {

    // 我们先只支持方向光
    struct DirectionalLight {
        Math::Vector3f direction;
        Math::Vector3f color;
        float intensity;
    };

    // 未来可以扩展到 PointLight, SpotLight 等
}
