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
        // �ӵ�λ����ʼ
        Math::Matrix4f transform = Math::Matrix4f::Identity();

        // ע��任˳�������ţ�����ת�����ƽ��
        // Model = Translate * Rotate * Scale

        if (j.contains("scale")) {
            // ���� scale ��һ��ͳһ�ĸ�����
            float s = j["scale"];
            // ����֧����������: Math::Vector3f s_vec = { j["scale"][0], ... };
            transform = transform * Math::Matrix4f::Scale({ s, s, s });
        }

        // if (j.contains("rotation")) { ... ��δʵ�� ... }

        if (j.contains("position")) {
            Math::Vector3f p = { j["position"][0], j["position"][1], j["position"][2] };
            // ע�⣺���ǵ� T*R*S Լ��Ҫ����Ӧ�����ź���ת������ƽ�ƾ���Ҫ��������߳�
            transform = Math::Matrix4f::Translate(p) * transform;
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
        if (data.contains("materials")) {
            for (const auto& mat_data : data["materials"]) {
                auto mat = std::make_shared<Renderer::Material>();
                std::string material_name = mat_data["name"];

                // --- ���ز��ʲ��� ---

                // 1. ���� Albedo ��ɫ���� (��ΪĬ��ֵ�����������)
                if (mat_data.contains("albedo_factor")) {
                    mat->albedo_factor = {
                        mat_data["albedo_factor"][0],
                        mat_data["albedo_factor"][1],
                        mat_data["albedo_factor"][2],
                        mat_data["albedo_factor"][3]
                    };
                }

                // 2. ���� Albedo ���� (���������ĺ����߼�)
                if (mat_data.contains("albedo_texture")) {
                    std::string texture_path = mat_data["albedo_texture"];

                    // ��������棬�����ظ�����
                    if (scene.m_textureCache.find(texture_path) == scene.m_textureCache.end()) {
                        // ���������û�У��ͼ����µ�����
                        scene.m_textureCache[texture_path] = Renderer::Texture::Load(texture_path);
                    }
                    mat->albedo_texture = scene.m_textureCache[texture_path];
                }

                // 3. �����������ղ���
                if (mat_data.contains("specular_shininess")) {
                    mat->specular_shininess = mat_data["specular_shininess"];
                }

                // --- ���� Shader (�ⲿ���߼�����) ---
                if (mat_data.contains("shader")) {
                    std::string shader_name = mat_data["shader"];
                    if (scene.m_shaderCache.find(shader_name) == scene.m_shaderCache.end()) {
                        if (s_shaderFactory.count(shader_name)) {
                            scene.m_shaderCache[shader_name] = s_shaderFactory[shader_name]();
                        }
                        else {
                            throw std::runtime_error("Shader not registered: " + shader_name);
                        }
                    }
                    mat->shader = scene.m_shaderCache[shader_name];
                }
                else {
                    // ���û��ָ��shader�������׳������ָ��һ��Ĭ�ϵ�
                    throw std::runtime_error("Material '" + material_name + "' does not specify a shader.");
                }

                // �����մ����õĲ��ʴ�����ʻ���
                scene.m_materialCache[material_name] = mat;
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

        if (data.contains("lights")) {
            for (const auto& light_data : data["lights"]) {
                if (light_data["type"] == "directional") {
                    DirectionalLight light;
                    light.direction = { light_data["direction"][0], light_data["direction"][1], light_data["direction"][2] };
                    // ��öԷ����������й�һ��
                    light.color = { light_data["color"][0], light_data["color"][1], light_data["color"][2] };
                    light.intensity = light_data["intensity"];
                    scene.m_directionalLights.push_back(light);
                }
            }
        }

        std::cout << "Scene loaded. " << scene.m_objects.size() << " objects." << std::endl;
        return scene;
    }
}
