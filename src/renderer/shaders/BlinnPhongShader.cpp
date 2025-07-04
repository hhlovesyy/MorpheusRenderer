#include "BlinnPhongShader.h"
#include "../Vertex.h"
#include "../../scene/Light.h" // 包含Light的定义
#include <iostream>
#include <algorithm>

namespace Morpheus::Renderer {

    Varyings BlinnPhongShader::VertexShader(const Vertex& in) {
        Varyings out;

        // 从 uniforms 获取矩阵
        auto model_matrix = std::any_cast<Math::Matrix4f>(uniforms["u_model"]);
        auto mvp_matrix = std::any_cast<Math::Matrix4f>(uniforms["u_mvp"]);

        // 1. 计算裁剪空间坐标
        out.position_clip = mvp_matrix * Math::Vector4f{ in.position.x(), in.position.y(), in.position.z(), 1.0f };
		out.uv = in.texCoords; // 传递纹理坐标
        // --- 构建 TBN 矩阵 ---
    // 1. 获取世界空间的 N, T
        Math::Vector3f T = Math::normalize((model_matrix * Math::Vector4f{ in.tangent.x(), in.tangent.y(), in.tangent.z(), 0.0f }).xyz());
        Math::Vector3f N = Math::normalize((std::any_cast<Math::Matrix4f>(uniforms["u_normal_matrix"]) * Math::Vector4f{ in.normal.x(), in.normal.y(), in.normal.z(), 0.0f }).xyz());
        // 2. 通过叉积重新计算 B，保证正交
        Math::Vector3f B = Math::normalize(Math::cross(N, T));

        // 3. 构建 TBN 矩阵 (从世界空间到切线空间)
        // 这是 view 矩阵的逆矩阵的转置。
        // 因为 TBN 矩阵是正交矩阵，所以它的逆等于它的转置。
        Math::Matrix3f TBN;
        /*TBN[0] = { T.x(), T.y(), T.z() };
        TBN[1] = { B.x(), B.y(), B.z() };
        TBN[2] = { N.x(), N.y(), N.z() };*/
        //todo：赋值太丑了，后面优化一下
		TBN[0][0] = T.x(); TBN[0][1] = T.y(); TBN[0][2] = T.z();
		TBN[1][0] = B.x(); TBN[1][1] = B.y(); TBN[1][2] = B.z();
		TBN[2][0] = N.x(); TBN[2][1] = N.y(); TBN[2][2] = N.z();

        // --- 将光照向量变换到切线空间 ---
        auto lights = std::any_cast<std::vector<Scene::DirectionalLight>>(uniforms["u_lights"]);
        if (!lights.empty()) {
            Math::Vector3f light_dir_world = Math::normalize(-lights[0].direction);
            out.tangent_space_light_dir = TBN * light_dir_world;
        }

        auto camera_pos = std::any_cast<Math::Vector3f>(uniforms["u_camera_pos"]);
        Math::Vector3f view_dir_world = Math::normalize(camera_pos - out.world_pos);
        out.tangent_space_view_dir = TBN * view_dir_world;
        return out;
    }

    Math::Vector4f BlinnPhongShader::FragmentShader(const Varyings& in) {
        // --- 1. 获取 Albedo 颜色 ---
        // 这是最终与光照结果相乘的基础色
        auto albedo_factor = std::any_cast<Math::Vector4f>(uniforms["u_albedo_factor"]);
        auto albedo_tex = std::any_cast<std::shared_ptr<Texture>>(uniforms["u_albedo_texture"]);
        Math::Vector4f albedo_color = albedo_factor; // 默认使用颜色因子
        if (albedo_tex) {
            albedo_color = albedo_tex->Sample(in.uv.x(), in.uv.y());
        }

        // --- 2. 获取法线 (核心修改) ---
        // 从法线贴图采样，如果不存在，则使用默认法线
        auto normal_tex = std::any_cast<std::shared_ptr<Texture>>(uniforms["u_normal_texture"]);
        Math::Vector3f tangent_space_normal;
        if (normal_tex) {
            // 从贴图采样，并将 [0, 1] 的颜色范围映射回 [-1, 1] 的法线向量范围
            tangent_space_normal = (normal_tex->Sample(in.uv.x(), in.uv.y()).xyz() * 2.0f) - Math::Vector3f{ 1.0f, 1.0f, 1.0f };
            tangent_space_normal = Math::normalize(tangent_space_normal);
        }
        else {
            // 如果没有法线贴图，就使用Z轴正方向作为法线。
            // 在切线空间中，(0,0,1) 就代表未经修改的、模型的几何法线方向。
            tangent_space_normal = { 0.0f, 0.0f, 1.0f };
        }

        // --- 3. 准备光照计算向量 (现在都在切线空间) ---
        Math::Vector3f normal = tangent_space_normal; // 我们用于光照的法线
        Math::Vector3f light_dir = Math::normalize(in.tangent_space_light_dir);
        Math::Vector3f view_dir = Math::normalize(in.tangent_space_view_dir);

        // --- 可视化调试 ---
        // a. 调试切线空间法线
        //return {(normal.x() + 1.0f) * 0.5f, (normal.y() + 1.0f) * 0.5f, (normal.z() + 1.0f) * 0.5f, 1.0f};

        // b. 调试UV坐标
        // return {in.uv.x(), in.uv.y(), 0.0f, 1.0f};

        // --- 4. 开始光照计算 ---
        auto lights = std::any_cast<std::vector<Scene::DirectionalLight>>(uniforms["u_lights"]);
        auto shininess = std::any_cast<float>(uniforms["u_shininess"]);

        Math::Vector3f ambient = { 0.1f, 0.1f, 0.1f }; // 环境光，可以作为 uniform 传入
        Math::Vector3f total_light_contribution = { 0.0f, 0.0f, 0.0f };

        for (const auto& light : lights) {
            // 漫反射 (Diffuse)
            float diff_factor = std::max(0.0f, Math::dot(normal, light_dir));
            Math::Vector3f diffuse = diff_factor * light.color * light.intensity;

            // 高光 (Specular - Blinn-Phong)
            Math::Vector3f halfway_dir = Math::normalize(light_dir + view_dir);
            float spec_factor = std::pow(std::max(0.0f, Math::dot(normal, halfway_dir)), shininess);
            Math::Vector3f specular = spec_factor * light.color * light.intensity;

            total_light_contribution = total_light_contribution + diffuse + specular;
        }

        // --- 5. 组合最终颜色 ---
        // 最终颜色 = (环境光 + 所有光源贡献) * 物体基础色
        Math::Vector3f final_color_rgb = (ambient + total_light_contribution);
        final_color_rgb.x() *= albedo_color.x();
        final_color_rgb.y() *= albedo_color.y();
        final_color_rgb.z() *= albedo_color.z();

        // 作为最后的保险，进行钳位，防止颜色过曝
        final_color_rgb.x() = std::clamp(final_color_rgb.x(), 0.0f, 1.0f);
        final_color_rgb.y() = std::clamp(final_color_rgb.y(), 0.0f, 1.0f);
        final_color_rgb.z() = std::clamp(final_color_rgb.z(), 0.0f, 1.0f);

        return { final_color_rgb.x(), final_color_rgb.y(), final_color_rgb.z(), albedo_color.w() };
    }
}
