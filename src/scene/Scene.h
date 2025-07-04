// src/scene/Scene.h (新文件)
#pragma once
#include "SceneObject.h"
#include "Camera.h"
#include "Light.h"
#include <vector>
#include <string>
#include <map>
#include <functional>

// 前向声明nlohmann::json
#include <nlohmann/json_fwd.hpp>
// 前向声明
namespace Morpheus::Renderer { class IShader; }

namespace Morpheus::Scene {

    class Scene {
    public:
        // 从一个JSON文件加载场景
        static Scene Load(const std::string& filepath);

        // --- Getters ---
        Camera& GetCamera(){ return m_camera; }
		const Camera& GetCamera() const { return m_camera; }
        const std::vector<SceneObject>& GetObjects() const { return m_objects; }
        // --- 新增一个静态方法用于注册 Shader ---
        static void RegisterShader(const std::string& name, std::function<std::shared_ptr<Renderer::IShader>()> factoryFn);
        const std::vector<DirectionalLight>& GetDirectionalLights() const { return m_directionalLights; }
    private:
        Scene() = default;

        Camera m_camera;
        std::vector<SceneObject> m_objects;
        std::vector<DirectionalLight> m_directionalLights;

        // 资源缓存，防止重复加载
        std::map<std::string, std::shared_ptr<Renderer::Mesh>> m_meshCache;
        std::map<std::string, std::shared_ptr<Renderer::Material>> m_materialCache;
        // --- 新增 Shader 缓存和创建工厂 ---
        std::map<std::string, std::shared_ptr<Renderer::IShader>> m_shaderCache;
        // Shader 注册表/工厂
        // 键是Shader的名字 (e.g., "Unlit", "PBR")
        // 值是一个能创建对应Shader实例的函数
        static std::map<std::string, std::function<std::shared_ptr<Renderer::IShader>()>> s_shaderFactory;
    };
}
