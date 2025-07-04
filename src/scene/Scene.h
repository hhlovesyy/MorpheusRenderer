// src/scene/Scene.h (新文件)
#pragma once
#include "SceneObject.h"
#include "Camera.h"
#include <vector>
#include <string>
#include <map>

// 前向声明nlohmann::json
#include <nlohmann/json_fwd.hpp>

namespace Morpheus::Scene {

    class Scene {
    public:
        // 从一个JSON文件加载场景
        static Scene Load(const std::string& filepath);

        // --- Getters ---
        Camera& GetCamera(){ return m_camera; }
		const Camera& GetCamera() const { return m_camera; }
        const std::vector<SceneObject>& GetObjects() const { return m_objects; }

    private:
        Scene() = default;

        Camera m_camera;
        std::vector<SceneObject> m_objects;

        // 资源缓存，防止重复加载
        std::map<std::string, std::shared_ptr<Renderer::Mesh>> m_meshCache;
        std::map<std::string, std::shared_ptr<Renderer::Material>> m_materialCache;
    };
}
