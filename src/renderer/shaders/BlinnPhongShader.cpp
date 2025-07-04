#include "BlinnPhongShader.h"
#include "../Vertex.h"
#include "../../scene/Light.h" // 包含Light的定义
#include <iostream>

namespace Morpheus::Renderer {

    Varyings BlinnPhongShader::VertexShader(const Vertex& in) {
        Varyings out;

        // 从 uniforms 获取矩阵
        auto model_matrix = std::any_cast<Math::Matrix4f>(uniforms["u_model"]);
        auto mvp_matrix = std::any_cast<Math::Matrix4f>(uniforms["u_mvp"]);

        // 1. 计算裁剪空间坐标
        out.position_clip = mvp_matrix * Math::Vector4f{ in.position.x(), in.position.y(), in.position.z(), 1.0f };

        // 2. 计算世界空间坐标
        // 注意：vec3(mat4 * vec4) 会自动丢弃w分量，如果你的数学库不支持，需要手动取.xyz()
        Math::Vector4f world_pos_v4 = model_matrix * Math::Vector4f{ in.position.x(), in.position.y(), in.position.z(), 1.0f };
        out.world_pos = { world_pos_v4.x(), world_pos_v4.y(), world_pos_v4.z() };

        // 3. 变换法线到世界空间
        // 使用法线矩阵: transpose(inverse(model_matrix))
        auto normal_matrix = std::any_cast<Math::Matrix4f>(uniforms["u_normal_matrix"]);
        Math::Vector4f world_normal_v4 = normal_matrix * Math::Vector4f{ in.normal.x(), in.normal.y(), in.normal.z(), 0.0f }; // 法线是方向，w=0
        out.world_normal = Math::normalize(Math::Vector3f{ world_normal_v4.x(), world_normal_v4.y(), world_normal_v4.z() });
		out.uv = in.texCoords; // 传递纹理坐标
        return out;
    }

    Math::Vector4f BlinnPhongShader::FragmentShader(const Varyings& in) {
        //查看世界空间的发现是否正确
        //打印out.world_normal,debug看看结果
        /*std::cout << "World Normal: ("
            << in.world_normal.x() << ", "
            << in.world_normal.y() << ", "
            << in.world_normal.z() << ")\n";*/
        //以下可以用来debug法线
       /* Math::Vector3f normal_color = (Math::normalize(in.world_normal) + Math::Vector3f{1.0f, 1.0f, 1.0f}) * 0.5f;
        return {normal_color.x(), normal_color.y(), normal_color.z(), 1.0f};*/

        // 从 uniforms 获取光照和材质参数
        Math::Vector4f albedo = std::any_cast<Math::Vector4f>(uniforms["u_albedo_factor"]);
        auto albedo_tex = std::any_cast<std::shared_ptr<Texture>>(uniforms["u_albedo_texture"]);
        if (albedo_tex) {
            albedo = albedo_tex->Sample(in.uv.x(), in.uv.y());
        }
        auto shininess = std::any_cast<float>(uniforms["u_shininess"]);
        auto camera_pos = std::any_cast<Math::Vector3f>(uniforms["u_camera_pos"]);
        auto lights = std::any_cast<std::vector<Scene::DirectionalLight>>(uniforms["u_lights"]);

        // --- 开始光照计算 ---

        // 1. 准备向量
        Math::Vector3f normal = Math::normalize(in.world_normal);
        Math::Vector3f view_dir = Math::normalize(camera_pos - in.world_pos);

        // 2. 环境光 (Ambient)
        Math::Vector3f ambient = { 0.3f, 0.3f, 0.3f }; // 硬编码一个环境光强度

        Math::Vector3f final_color_rgb = { 0.0f, 0.0f, 0.0f };

        // 3. 对每个光源进行迭代
        for (const auto& light : lights) {
            Math::Vector3f light_dir = Math::normalize(-light.direction); // 指向光源的方向

            // 4. 漫反射 (Diffuse)
            float diff_factor = std::max(0.0f, Math::dot(normal, light_dir));
            //直接return diff_factor，看看效果
			//return { diff_factor, diff_factor, diff_factor, 1.0f };
  
            Math::Vector3f diffuse = diff_factor * light.color * light.intensity;

            // 5. 高光 (Specular - Blinn-Phong)
            Math::Vector3f halfway_dir = Math::normalize(light_dir + view_dir);
            float spec_factor = std::pow(std::max(0.0f, Math::dot(normal, halfway_dir)), shininess);
			//return { spec_factor, spec_factor, spec_factor, 1.0f }; // 直接返回 spec_factor 看看效果
            Math::Vector3f specular = spec_factor * light.color * light.intensity;
			//return { specular.x(), specular.y(), specular.z(), 1.0f }; // 直接返回 specular 看看效果
            final_color_rgb = final_color_rgb + diffuse + specular;
        }

        // 最终颜色 = (环境光 + 光照结果) * 物体基础色
        final_color_rgb = (ambient + final_color_rgb);
        final_color_rgb.x() *= albedo.x();
        final_color_rgb.y() *= albedo.y();
        final_color_rgb.z() *= albedo.z();

        //输出一下值
        /*std::cout << "Final Color RGB: (" 
                  << final_color_rgb.x() << ", " 
                  << final_color_rgb.y() << ", " 
			<< final_color_rgb.z() << ")\n";*/

        return { final_color_rgb.x(), final_color_rgb.y(), final_color_rgb.z(), albedo.w() };
    }
}
