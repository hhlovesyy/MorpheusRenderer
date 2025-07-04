#pragma once
#include <memory>
#include "Framebuffer.h"
#include "Mesh.h"
#include "../scene/Scene.h"
#include "../math/Matrix.h"

namespace Morpheus::Renderer {
    struct Vertex {
        Math::Vector3f position;
        Math::Vector3f normal; // <--- 新增法线
        Math::Vector2f texCoords;
        Math::Vector3f tangent; // <--- 新增
    };
}