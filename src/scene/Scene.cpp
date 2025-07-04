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
        // 从单位矩阵开始
        Math::Matrix4f transform = Math::Matrix4f::Identity();

        // 注意变换顺序：先缩放，再旋转，最后平移
        // Model = Translate * Rotate * Scale

        if (j.contains("scale")) {
            // 假设 scale 是一个统一的浮点数
            float s = j["scale"];
            // 或者支持三轴缩放: Math::Vector3f s_vec = { j["scale"][0], ... };
            transform = transform * Math::Matrix4f::Scale({ s, s, s });
        }

        // if (j.contains("rotation")) { ... 暂未实现 ... }

        if (j.contains("position")) {
            Math::Vector3f p = { j["position"][0], j["position"][1], j["position"][2] };
            // 注意：我们的 T*R*S 约定要求先应用缩放和旋转，所以平移矩阵要放在最左边乘
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
        if (data.contains("materials")) {
            for (const auto& mat_data : data["materials"]) {
                auto mat = std::make_shared<Renderer::Material>();
                std::string material_name = mat_data["name"];

                // --- 加载材质参数 ---

                // 1. 加载 Albedo 颜色因子 (作为默认值或与纹理相乘)
                if (mat_data.contains("albedo_factor")) {
                    mat->albedo_factor = {
                        mat_data["albedo_factor"][0],
                        mat_data["albedo_factor"][1],
                        mat_data["albedo_factor"][2],
                        mat_data["albedo_factor"][3]
                    };
                }

                // 2. 加载 Albedo 纹理 (这是新增的核心逻辑)
                if (mat_data.contains("albedo_texture")) {
                    std::string texture_path = mat_data["albedo_texture"];

                    // 检查纹理缓存，避免重复加载
                    if (scene.m_textureCache.find(texture_path) == scene.m_textureCache.end()) {
                        // 如果缓存中没有，就加载新的纹理
                        scene.m_textureCache[texture_path] = Renderer::Texture::Load(texture_path);
                    }
                    mat->albedo_texture = scene.m_textureCache[texture_path];
                }

                // 3. 加载其他光照参数
                if (mat_data.contains("specular_shininess")) {
                    mat->specular_shininess = mat_data["specular_shininess"];
                }

                // --- 关联 Shader (这部分逻辑不变) ---
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
                    // 如果没有指定shader，可以抛出错误或指定一个默认的
                    throw std::runtime_error("Material '" + material_name + "' does not specify a shader.");
                }

                // 将最终创建好的材质存入材质缓存
                scene.m_materialCache[material_name] = mat;
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

        if (data.contains("lights")) {
            for (const auto& light_data : data["lights"]) {
                if (light_data["type"] == "directional") {
                    DirectionalLight light;
                    light.direction = { light_data["direction"][0], light_data["direction"][1], light_data["direction"][2] };
                    // 最好对方向向量进行归一化
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
