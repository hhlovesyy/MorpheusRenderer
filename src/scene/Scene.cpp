// src/scene/Scene.cpp (新文件)
#include "Scene.h"
#include <fstream>
#include <sstream>
#include <nlohmann/json.hpp>
#include <iostream>
#include "../renderer/IShader.h"
#include <SDL.h>

using json = nlohmann::json;
// 初始化静态成员
std::map<std::string, std::function<std::shared_ptr<Morpheus::Renderer::IShader>()>> Morpheus::Scene::Scene::s_shaderFactory;

namespace Morpheus::Scene {
    void Scene::RegisterShader(const std::string& name, std::function<std::shared_ptr<Renderer::IShader>()> factoryFn) {
        s_shaderFactory[name] = factoryFn;
    }
    // 我们需要一个辅助函数来解析矩阵
    Math::Matrix4f parse_transform(const json& j) {
        Math::Matrix4f transform = Math::Matrix4f::Identity();
        // 这里只是一个简化的例子，你可以扩展它来支持 position, rotation, scale
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

        // --- 在这里添加调试代码 ---
		std::stringstream buffer;
        buffer << f.rdbuf();
        std::string file_content = buffer.str();

        // 使用SDL_Log在VS的输出窗口打印，或者直接设置断点查看
        SDL_Log("--- Start of Scene File Content ---");
        SDL_Log(file_content.c_str());
        SDL_Log("--- End of Scene File Content ---");

        // 将文件流的读取位置重置到开头
        f.seekg(0);
        // -------------------------

        json data = json::parse(f);

        // 加载相机设置
        if (data.contains("camera")) {
            const auto& cam_data = data["camera"];
            Math::Vector3f pos = { cam_data["position"][0], cam_data["position"][1], cam_data["position"][2] };
            Math::Vector3f focal = { cam_data["focal_point"][0], cam_data["focal_point"][1], cam_data["focal_point"][2] };
            scene.m_camera.SetPosition(pos);
            scene.m_camera.SetFocalPoint(focal);
            scene.m_camera.UpdateViewMatrix();
        }

        // 加载材质
        if (data.contains("materials"))
        {
            for (const auto& mat_data : data["materials"])
            {
                auto mat = std::make_shared<Renderer::Material>();
                mat->color = { mat_data["color"][0], mat_data["color"][1], mat_data["color"][2], mat_data["color"][3] };

                // --- 从工厂创建或从缓存获取 Shader ---
                std::string shader_name = mat_data["shader"];
                if (scene.m_shaderCache.find(shader_name) == scene.m_shaderCache.end()) {
                    // 如果缓存中没有，就从工厂创建一个新的
                    if (s_shaderFactory.count(shader_name)) {
                        scene.m_shaderCache[shader_name] = s_shaderFactory[shader_name]();
                    }
                    else {
                        // 抛出错误或使用一个默认的fallback shader
                        throw std::runtime_error("Shader not registered: " + shader_name);
                    }
                }
                mat->shader = scene.m_shaderCache[shader_name]; // 关联 Shader

                scene.m_materialCache[mat_data["name"]] = mat;
            }
        }

        // 加载场景对象
        if (data.contains("objects")) {
            for (const auto& obj_data : data["objects"]) {
                SceneObject obj;
                obj.name = obj_data["name"];
                obj.transform = parse_transform(obj_data["transform"]);

                // 从缓存或文件加载Mesh
                std::string mesh_path = obj_data["mesh"];
                if (scene.m_meshCache.find(mesh_path) == scene.m_meshCache.end()) {
                    scene.m_meshCache[mesh_path] = std::make_shared<Renderer::Mesh>(Renderer::Mesh::LoadFromObj(mesh_path));
                }
                obj.mesh = scene.m_meshCache[mesh_path];

                // 从缓存查找Material
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
