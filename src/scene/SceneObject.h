// src/scene/SceneObject.h (���ļ�)
#pragma once
#include "../math/Matrix.h"
#include "../renderer/Mesh.h"
#include "../renderer/Material.h"
#include <memory>
#include <string>

namespace Morpheus::Scene {
    
    class SceneObject {
    public:
        std::string name;
        Math::Matrix4f transform; // ģ�ͱ任����
        std::shared_ptr<Renderer::Mesh> mesh;
        std::shared_ptr<Renderer::Material> material;
        
        SceneObject() : transform(Math::Matrix4f::Identity()) {}
    };
}
