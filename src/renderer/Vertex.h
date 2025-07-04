#pragma once
#include <memory>
#include "Framebuffer.h"
#include "Mesh.h"
#include "../scene/Scene.h"
#include "../math/Matrix.h"

namespace Morpheus::Renderer {
    struct Vertex {
        Math::Vector3f position;
        // Math::Vector3f normal; // 为未来光照做准备
        // Math::Vector2f texCoords;
    };
}