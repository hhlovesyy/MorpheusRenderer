#include "BlinnPhongShader.h"
#include "../Vertex.h"
#include "../../scene/Light.h" // ����Light�Ķ���
#include <iostream>
#include <algorithm>

namespace Morpheus::Renderer {

    Varyings BlinnPhongShader::VertexShader(const Vertex& in) {
        Varyings out;

        // �� uniforms ��ȡ����
        auto model_matrix = std::any_cast<Math::Matrix4f>(uniforms["u_model"]);
        auto mvp_matrix = std::any_cast<Math::Matrix4f>(uniforms["u_mvp"]);

        // 1. ����ü��ռ�����
        out.position_clip = mvp_matrix * Math::Vector4f{ in.position.x(), in.position.y(), in.position.z(), 1.0f };
		out.uv = in.texCoords; // ������������
        // --- ���� TBN ���� ---
    // 1. ��ȡ����ռ�� N, T
        Math::Vector3f T = Math::normalize((model_matrix * Math::Vector4f{ in.tangent.x(), in.tangent.y(), in.tangent.z(), 0.0f }).xyz());
        Math::Vector3f N = Math::normalize((std::any_cast<Math::Matrix4f>(uniforms["u_normal_matrix"]) * Math::Vector4f{ in.normal.x(), in.normal.y(), in.normal.z(), 0.0f }).xyz());
        // 2. ͨ��������¼��� B����֤����
        Math::Vector3f B = Math::normalize(Math::cross(N, T));

        // 3. ���� TBN ���� (������ռ䵽���߿ռ�)
        // ���� view �����������ת�á�
        // ��Ϊ TBN �������������������������������ת�á�
        Math::Matrix3f TBN;
        /*TBN[0] = { T.x(), T.y(), T.z() };
        TBN[1] = { B.x(), B.y(), B.z() };
        TBN[2] = { N.x(), N.y(), N.z() };*/
        //todo����ֵ̫���ˣ������Ż�һ��
		TBN[0][0] = T.x(); TBN[0][1] = T.y(); TBN[0][2] = T.z();
		TBN[1][0] = B.x(); TBN[1][1] = B.y(); TBN[1][2] = B.z();
		TBN[2][0] = N.x(); TBN[2][1] = N.y(); TBN[2][2] = N.z();

        // --- �����������任�����߿ռ� ---
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
        // --- 1. ��ȡ Albedo ��ɫ ---
        // ������������ս����˵Ļ���ɫ
        auto albedo_factor = std::any_cast<Math::Vector4f>(uniforms["u_albedo_factor"]);
        auto albedo_tex = std::any_cast<std::shared_ptr<Texture>>(uniforms["u_albedo_texture"]);
        Math::Vector4f albedo_color = albedo_factor; // Ĭ��ʹ����ɫ����
        if (albedo_tex) {
            albedo_color = albedo_tex->Sample(in.uv.x(), in.uv.y());
        }

        // --- 2. ��ȡ���� (�����޸�) ---
        // �ӷ�����ͼ��������������ڣ���ʹ��Ĭ�Ϸ���
        auto normal_tex = std::any_cast<std::shared_ptr<Texture>>(uniforms["u_normal_texture"]);
        Math::Vector3f tangent_space_normal;
        if (normal_tex) {
            // ����ͼ���������� [0, 1] ����ɫ��Χӳ��� [-1, 1] �ķ���������Χ
            tangent_space_normal = (normal_tex->Sample(in.uv.x(), in.uv.y()).xyz() * 2.0f) - Math::Vector3f{ 1.0f, 1.0f, 1.0f };
            tangent_space_normal = Math::normalize(tangent_space_normal);
        }
        else {
            // ���û�з�����ͼ����ʹ��Z����������Ϊ���ߡ�
            // �����߿ռ��У�(0,0,1) �ʹ���δ���޸ĵġ�ģ�͵ļ��η��߷���
            tangent_space_normal = { 0.0f, 0.0f, 1.0f };
        }

        // --- 3. ׼�����ռ������� (���ڶ������߿ռ�) ---
        Math::Vector3f normal = tangent_space_normal; // �������ڹ��յķ���
        Math::Vector3f light_dir = Math::normalize(in.tangent_space_light_dir);
        Math::Vector3f view_dir = Math::normalize(in.tangent_space_view_dir);

        // --- ���ӻ����� ---
        // a. �������߿ռ䷨��
        //return {(normal.x() + 1.0f) * 0.5f, (normal.y() + 1.0f) * 0.5f, (normal.z() + 1.0f) * 0.5f, 1.0f};

        // b. ����UV����
        // return {in.uv.x(), in.uv.y(), 0.0f, 1.0f};

        // --- 4. ��ʼ���ռ��� ---
        auto lights = std::any_cast<std::vector<Scene::DirectionalLight>>(uniforms["u_lights"]);
        auto shininess = std::any_cast<float>(uniforms["u_shininess"]);

        Math::Vector3f ambient = { 0.1f, 0.1f, 0.1f }; // �����⣬������Ϊ uniform ����
        Math::Vector3f total_light_contribution = { 0.0f, 0.0f, 0.0f };

        for (const auto& light : lights) {
            // ������ (Diffuse)
            float diff_factor = std::max(0.0f, Math::dot(normal, light_dir));
            Math::Vector3f diffuse = diff_factor * light.color * light.intensity;

            // �߹� (Specular - Blinn-Phong)
            Math::Vector3f halfway_dir = Math::normalize(light_dir + view_dir);
            float spec_factor = std::pow(std::max(0.0f, Math::dot(normal, halfway_dir)), shininess);
            Math::Vector3f specular = spec_factor * light.color * light.intensity;

            total_light_contribution = total_light_contribution + diffuse + specular;
        }

        // --- 5. ���������ɫ ---
        // ������ɫ = (������ + ���й�Դ����) * �������ɫ
        Math::Vector3f final_color_rgb = (ambient + total_light_contribution);
        final_color_rgb.x() *= albedo_color.x();
        final_color_rgb.y() *= albedo_color.y();
        final_color_rgb.z() *= albedo_color.z();

        // ��Ϊ���ı��գ�����ǯλ����ֹ��ɫ����
        final_color_rgb.x() = std::clamp(final_color_rgb.x(), 0.0f, 1.0f);
        final_color_rgb.y() = std::clamp(final_color_rgb.y(), 0.0f, 1.0f);
        final_color_rgb.z() = std::clamp(final_color_rgb.z(), 0.0f, 1.0f);

        return { final_color_rgb.x(), final_color_rgb.y(), final_color_rgb.z(), albedo_color.w() };
    }
}
