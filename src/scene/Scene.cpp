// src/scene/Scene.cpp (���ļ�)
#include "Scene.h"
#include <fstream>
#include <sstream>
#include <nlohmann/json.hpp>
#include <iostream>
#include "../renderer/IShader.h"
#include <SDL.h>

using json = nlohmann::json;
// ��ʼ����̬��Ա
std::map<std::string, std::function<std::shared_ptr<Morpheus::Renderer::IShader>()>> Morpheus::Scene::Scene::s_shaderFactory;

namespace Morpheus::Scene {
    void Scene::RegisterShader(const std::string& name, std::function<std::shared_ptr<Renderer::IShader>()> factoryFn) {
        s_shaderFactory[name] = factoryFn;
    }
    // ������Ҫһ��������������������
    Math::Matrix4f parse_transform(const json& j) {
        Math::Matrix4f transform = Math::Matrix4f::Identity();
        // ����ֻ��һ���򻯵����ӣ��������չ����֧�� position, rotation, scale
        if (j.contains("scale")) {
            float s = j["scale"];
            transform.m[0][0] = s;
            transform.m[1][1] = s;
            transform.m[2][2] = s;
        }
        if (j.contains("position")) {
            transform.m[0][3] = j["position"][0];
            transform.m[1][3] = j["position"][1];
            transform.m[2][3] = j["position"][2];
        }
        return transform;
    }

    Scene Scene::Load(const std::string& filepath) {
        Scene scene;
        std::ifstream f(filepath);
        if (!f.is_open()) {
            throw std::runtime_error("Failed to open scene file: " + filepath);
        }

        // --- ��������ӵ��Դ��� ---
		std::stringstream buffer;
        buffer << f.rdbuf();
        std::string file_content = buffer.str();

        // ʹ��SDL_Log��VS��������ڴ�ӡ������ֱ�����öϵ�鿴
        SDL_Log("--- Start of Scene File Content ---");
        SDL_Log(file_content.c_str());
        SDL_Log("--- End of Scene File Content ---");

        // ���ļ����Ķ�ȡλ�����õ���ͷ
        f.seekg(0);
        // -------------------------

        json data = json::parse(f);

        // �����������
        if (data.contains("camera")) {
            const auto& cam_data = data["camera"];
            Math::Vector3f pos = { cam_data["position"][0], cam_data["position"][1], cam_data["position"][2] };
            Math::Vector3f focal = { cam_data["focal_point"][0], cam_data["focal_point"][1], cam_data["focal_point"][2] };
            scene.m_camera.SetPosition(pos);
            scene.m_camera.SetFocalPoint(focal);
            scene.m_camera.UpdateViewMatrix();
        }

        // ���ز���
        if (data.contains("materials"))
        {
            for (const auto& mat_data : data["materials"])
            {
                auto mat = std::make_shared<Renderer::Material>();
                mat->color = { mat_data["color"][0], mat_data["color"][1], mat_data["color"][2], mat_data["color"][3] };

                // --- �ӹ���������ӻ����ȡ Shader ---
                std::string shader_name = mat_data["shader"];
                if (scene.m_shaderCache.find(shader_name) == scene.m_shaderCache.end()) {
                    // ���������û�У��ʹӹ�������һ���µ�
                    if (s_shaderFactory.count(shader_name)) {
                        scene.m_shaderCache[shader_name] = s_shaderFactory[shader_name]();
                    }
                    else {
                        // �׳������ʹ��һ��Ĭ�ϵ�fallback shader
                        throw std::runtime_error("Shader not registered: " + shader_name);
                    }
                }
                mat->shader = scene.m_shaderCache[shader_name]; // ���� Shader

                scene.m_materialCache[mat_data["name"]] = mat;
            }
        }

        // ���س�������
        if (data.contains("objects")) {
            for (const auto& obj_data : data["objects"]) {
                SceneObject obj;
                obj.name = obj_data["name"];
                obj.transform = parse_transform(obj_data["transform"]);

                // �ӻ�����ļ�����Mesh
                std::string mesh_path = obj_data["mesh"];
                if (scene.m_meshCache.find(mesh_path) == scene.m_meshCache.end()) {
                    scene.m_meshCache[mesh_path] = std::make_shared<Renderer::Mesh>(Renderer::Mesh::LoadFromObj(mesh_path));
                }
                obj.mesh = scene.m_meshCache[mesh_path];

                // �ӻ������Material
                std::string mat_name = obj_data["material"];
                if (scene.m_materialCache.find(mat_name) != scene.m_materialCache.end()) {
                    obj.material = scene.m_materialCache[mat_name];
                }

                scene.m_objects.push_back(obj);
            }
        }

        std::cout << "Scene loaded. " << scene.m_objects.size() << " objects." << std::endl;
        return scene;
    }
}
