#pragma once
#include <memory>
#include "Framebuffer.h"
#include "Mesh.h"
#include "../scene/Scene.h"
#include "../math/Matrix.h"

namespace Morpheus::Renderer {
    struct Vertex {
        Math::Vector3f position;
        Math::Vector3f normal; // <--- ��������
        Math::Vector2f texCoords;
        Math::Vector3f tangent; // <--- ����
    };
}