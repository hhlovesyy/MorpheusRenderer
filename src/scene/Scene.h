// src/scene/Scene.h (���ļ�)
#pragma once
#include "SceneObject.h"
#include "Camera.h"
#include "Light.h"
#include <vector>
#include <string>
#include <map>
#include <functional>

// ǰ������nlohmann::json
#include <nlohmann/json_fwd.hpp>
// ǰ������
namespace Morpheus::Renderer { class IShader; }

namespace Morpheus::Scene {

    class Scene {
    public:
        // ��һ��JSON�ļ����س���
        static Scene Load(const std::string& filepath);

        // --- Getters ---
        Camera& GetCamera(){ return m_camera; }
		const Camera& GetCamera() const { return m_camera; }
        const std::vector<SceneObject>& GetObjects() const { return m_objects; }
        // --- ����һ����̬��������ע�� Shader ---
        static void RegisterShader(const std::string& name, std::function<std::shared_ptr<Renderer::IShader>()> factoryFn);
        const std::vector<DirectionalLight>& GetDirectionalLights() const { return m_directionalLights; }
    private:
        Scene() = default;

        Camera m_camera;
        std::vector<SceneObject> m_objects;
        std::vector<DirectionalLight> m_directionalLights;

        // ��Դ���棬��ֹ�ظ�����
        std::map<std::string, std::shared_ptr<Renderer::Mesh>> m_meshCache;
        std::map<std::string, std::shared_ptr<Renderer::Material>> m_materialCache;
        // --- ���� Shader ����ʹ������� ---
        std::map<std::string, std::shared_ptr<Renderer::IShader>> m_shaderCache;
        // Shader ע���/����
        // ����Shader������ (e.g., "Unlit", "PBR")
        // ֵ��һ���ܴ�����ӦShaderʵ���ĺ���
        static std::map<std::string, std::function<std::shared_ptr<Renderer::IShader>()>> s_shaderFactory;
    };
}
