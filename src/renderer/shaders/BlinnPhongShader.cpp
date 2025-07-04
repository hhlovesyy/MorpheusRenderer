#include "BlinnPhongShader.h"
#include "../Vertex.h"
#include "../../scene/Light.h" // ����Light�Ķ���
#include <iostream>

namespace Morpheus::Renderer {

    Varyings BlinnPhongShader::VertexShader(const Vertex& in) {
        Varyings out;

        // �� uniforms ��ȡ����
        auto model_matrix = std::any_cast<Math::Matrix4f>(uniforms["u_model"]);
        auto mvp_matrix = std::any_cast<Math::Matrix4f>(uniforms["u_mvp"]);

        // 1. ����ü��ռ�����
        out.position_clip = mvp_matrix * Math::Vector4f{ in.position.x(), in.position.y(), in.position.z(), 1.0f };

        // 2. ��������ռ�����
        // ע�⣺vec3(mat4 * vec4) ���Զ�����w��������������ѧ�ⲻ֧�֣���Ҫ�ֶ�ȡ.xyz()
        Math::Vector4f world_pos_v4 = model_matrix * Math::Vector4f{ in.position.x(), in.position.y(), in.position.z(), 1.0f };
        out.world_pos = { world_pos_v4.x(), world_pos_v4.y(), world_pos_v4.z() };

        // 3. �任���ߵ�����ռ�
        // ʹ�÷��߾���: transpose(inverse(model_matrix))
        auto normal_matrix = std::any_cast<Math::Matrix4f>(uniforms["u_normal_matrix"]);
        Math::Vector4f world_normal_v4 = normal_matrix * Math::Vector4f{ in.normal.x(), in.normal.y(), in.normal.z(), 0.0f }; // �����Ƿ���w=0
        out.world_normal = Math::normalize(Math::Vector3f{ world_normal_v4.x(), world_normal_v4.y(), world_normal_v4.z() });
		out.uv = in.texCoords; // ������������
        return out;
    }

    Math::Vector4f BlinnPhongShader::FragmentShader(const Varyings& in) {
        //�鿴����ռ�ķ����Ƿ���ȷ
        //��ӡout.world_normal,debug�������
        /*std::cout << "World Normal: ("
            << in.world_normal.x() << ", "
            << in.world_normal.y() << ", "
            << in.world_normal.z() << ")\n";*/
        //���¿�������debug����
       /* Math::Vector3f normal_color = (Math::normalize(in.world_normal) + Math::Vector3f{1.0f, 1.0f, 1.0f}) * 0.5f;
        return {normal_color.x(), normal_color.y(), normal_color.z(), 1.0f};*/

        // �� uniforms ��ȡ���պͲ��ʲ���
        Math::Vector4f albedo = std::any_cast<Math::Vector4f>(uniforms["u_albedo_factor"]);
        auto albedo_tex = std::any_cast<std::shared_ptr<Texture>>(uniforms["u_albedo_texture"]);
        if (albedo_tex) {
            albedo = albedo_tex->Sample(in.uv.x(), in.uv.y());
        }
        auto shininess = std::any_cast<float>(uniforms["u_shininess"]);
        auto camera_pos = std::any_cast<Math::Vector3f>(uniforms["u_camera_pos"]);
        auto lights = std::any_cast<std::vector<Scene::DirectionalLight>>(uniforms["u_lights"]);

        // --- ��ʼ���ռ��� ---

        // 1. ׼������
        Math::Vector3f normal = Math::normalize(in.world_normal);
        Math::Vector3f view_dir = Math::normalize(camera_pos - in.world_pos);

        // 2. ������ (Ambient)
        Math::Vector3f ambient = { 0.3f, 0.3f, 0.3f }; // Ӳ����һ��������ǿ��

        Math::Vector3f final_color_rgb = { 0.0f, 0.0f, 0.0f };

        // 3. ��ÿ����Դ���е���
        for (const auto& light : lights) {
            Math::Vector3f light_dir = Math::normalize(-light.direction); // ָ���Դ�ķ���

            // 4. ������ (Diffuse)
            float diff_factor = std::max(0.0f, Math::dot(normal, light_dir));
            //ֱ��return diff_factor������Ч��
			//return { diff_factor, diff_factor, diff_factor, 1.0f };
  
            Math::Vector3f diffuse = diff_factor * light.color * light.intensity;

            // 5. �߹� (Specular - Blinn-Phong)
            Math::Vector3f halfway_dir = Math::normalize(light_dir + view_dir);
            float spec_factor = std::pow(std::max(0.0f, Math::dot(normal, halfway_dir)), shininess);
			//return { spec_factor, spec_factor, spec_factor, 1.0f }; // ֱ�ӷ��� spec_factor ����Ч��
            Math::Vector3f specular = spec_factor * light.color * light.intensity;
			//return { specular.x(), specular.y(), specular.z(), 1.0f }; // ֱ�ӷ��� specular ����Ч��
            final_color_rgb = final_color_rgb + diffuse + specular;
        }

        // ������ɫ = (������ + ���ս��) * �������ɫ
        final_color_rgb = (ambient + final_color_rgb);
        final_color_rgb.x() *= albedo.x();
        final_color_rgb.y() *= albedo.y();
        final_color_rgb.z() *= albedo.z();

        //���һ��ֵ
        /*std::cout << "Final Color RGB: (" 
                  << final_color_rgb.x() << ", " 
                  << final_color_rgb.y() << ", " 
			<< final_color_rgb.z() << ")\n";*/

        return { final_color_rgb.x(), final_color_rgb.y(), final_color_rgb.z(), albedo.w() };
    }
}
