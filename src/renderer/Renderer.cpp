// src/renderer/Renderer.cpp
#include "Renderer.h"
#include "../math/Common.h" // <--- �����µ�ͷ�ļ�
#include <algorithm>
#include <vector>
#include <cmath>
#include <iostream>
#include "../scene/Scene.h"
#include "IShader.h"
#include "../renderer/Clipping.h"
#include <SDL.h>

namespace Morpheus::Renderer {
    // ... Renderer::Renderer(...) �� Renderer::DrawLine(...) �������ֲ��� ...

    Renderer::Renderer(int width, int height) {
        m_framebuffer = std::make_shared<Framebuffer>(width, height);
    }

    void Renderer::RasterizeTriangle(const Varyings& v0, const Varyings& v1, const Varyings& v2, IShader& shader) {
        // 1. �ӿڱ任 (��֮ǰ��ͬ��������Varyings)
        int w = m_framebuffer->GetWidth();
        int h = m_framebuffer->GetHeight();
        Math::vec<3, float> p0_screen = { (v0.position_clip.x() + 1.0f) * 0.5f * w, (v0.position_clip.y() + 1.0f) * 0.5f * h, v0.position_clip.z() };
        Math::vec<3, float> p1_screen = { (v1.position_clip.x() + 1.0f) * 0.5f * w, (v1.position_clip.y() + 1.0f) * 0.5f * h, v1.position_clip.z() };
        Math::vec<3, float> p2_screen = { (v2.position_clip.x() + 1.0f) * 0.5f * w, (v2.position_clip.y() + 1.0f) * 0.5f * h, v2.position_clip.z() };

        // 2. �����Χ�� (��֮ǰ��ͬ)
        int minX = std::max(0, (int)std::floor(std::min({ p0_screen.x(), p1_screen.x(), p2_screen.x() })));
        int minY = std::max(0, (int)std::floor(std::min({ p0_screen.y(), p1_screen.y(), p2_screen.y() })));
        int maxX = std::min(w - 1, (int)std::ceil(std::max({ p0_screen.x(), p1_screen.x(), p2_screen.x() })));
        int maxY = std::min(h - 1, (int)std::ceil(std::max({ p0_screen.y(), p1_screen.y(), p2_screen.y() })));

        // 3. ������Χ���ڵ�ÿ������
        for (int y = minY; y <= maxY; ++y) {
            for (int x = minX; x <= maxX; ++x) {
                // ���㵱ǰ�������ĵ���������
                Math::vec<2, float> p = { (float)x + 0.5f, (float)y + 0.5f };
                Math::vec<2, float> pa = { p0_screen.x(), p0_screen.y() };
                Math::vec<2, float> pb = { p1_screen.x(), p1_screen.y() };
                Math::vec<2, float> pc = { p2_screen.x(), p2_screen.y() };

                float total_area = (pb.x() - pa.x()) * (pc.y() - pa.y()) - (pc.x() - pa.x()) * (pb.y() - pa.y());
                float w0 = ((pb.x() - p.x()) * (pc.y() - p.y()) - (pc.x() - p.x()) * (pb.y() - p.y())) / total_area;
                float w1 = ((pc.x() - p.x()) * (pa.y() - p.y()) - (pa.x() - p.x()) * (pc.y() - p.y())) / total_area;
                float w2 = 1.0f - w0 - w1;

                // ����������궼��[0,1]��Χ�ڣ�����������������
                if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
                    float one_over_w0 = 1.0f / v0.position_clip.w();
                    float one_over_w1 = 1.0f / v1.position_clip.w();
                    float one_over_w2 = 1.0f / v2.position_clip.w();
                    float one_over_w_interp = w0 * one_over_w0 + w1 * one_over_w1 + w2 * one_over_w2;
                    float w_interp = 1.0f / one_over_w_interp;

                    // ��ֵ���
                    float z_interp = (p0_screen.z() * w0 + p1_screen.z() * w1 + p2_screen.z() * w2);

                    // --- �����޸ģ���ֵ���� Varyings ---
                    Varyings interpolated_varyings;
                    // �� color/w ���в�ֵ
                    interpolated_varyings.color = ((v0.color * one_over_w0) * w0 +
                        (v1.color * one_over_w1) * w1 +
                        (v2.color * one_over_w2) * w2) * w_interp;

                    interpolated_varyings.world_normal = ((v0.world_normal * one_over_w0) * w0 +
                        (v1.world_normal * one_over_w1) * w1 +
                        (v2.world_normal * one_over_w2) * w2) * w_interp;

                    interpolated_varyings.uv = ((v0.uv * one_over_w0) * w0 +
                        (v1.uv * one_over_w1) * w1 +
						(v2.uv * one_over_w2) * w2) * w_interp;

                    interpolated_varyings.tangent_space_light_dir = ((v0.tangent_space_light_dir * one_over_w0) * w0 +
                        (v1.tangent_space_light_dir * one_over_w1) * w1 +
						(v2.tangent_space_light_dir * one_over_w2) * w2) * w_interp;
                    interpolated_varyings.tangent_space_view_dir = ((v0.tangent_space_view_dir * one_over_w0) * w0 +
						(v1.tangent_space_view_dir * one_over_w1) * w1 + (v2.tangent_space_view_dir * one_over_w2) * w2) * w_interp;

                    // 5. ����ƬԪ��ɫ��
                    Math::Vector4f final_color = shader.FragmentShader(interpolated_varyings);

                    // 6. д��֡���� (��Ȳ���)
                    m_framebuffer->SetPixel(x, y, z_interp, final_color);
                }
            }
        }
    }

    // --- Render �����������ع� ---
    void Renderer::Render(const Scene::Scene& scene) {
        m_framebuffer->ClearColor({ 0.1f, 0.1f, 0.1f, 1.0f });
        m_framebuffer->ClearDepth(1.0f);

        const auto& camera = scene.GetCamera();
        const Math::Matrix4f& viewMatrix = camera.GetViewMatrix();
        const Math::Matrix4f& projectionMatrix = camera.GetProjectionMatrix();

        for (const auto& object : scene.GetObjects()) {
            if (!object.mesh || !object.material || !object.material->shader) {
                SDL_Log("Skipping object '%s': mesh or material or shader is null", object.name.c_str());
                continue;
            }

            auto& shader = *object.material->shader;
            const auto& material = *object.material;

            // 1. ���� Shader Uniforms
            const Math::Matrix4f& modelMatrix = object.transform;
            shader.uniforms["u_model"] = modelMatrix;
            shader.uniforms["u_view"] = viewMatrix;
            shader.uniforms["u_projection"] = projectionMatrix;
            shader.uniforms["u_mvp"] = projectionMatrix * viewMatrix * modelMatrix;

            // --- ������ Uniforms ---
            // ���㲢���ݷ��߾���
            shader.uniforms["u_normal_matrix"] = modelMatrix.inverse().transpose();

            // ���ݲ��ʲ���
            shader.uniforms["u_albedo_factor"] = material.albedo_factor;
			shader.uniforms["u_albedo_texture"] = material.albedo_texture;
			shader.uniforms["u_normal_texture"] = material.normal_texture;
            shader.uniforms["u_shininess"] = material.specular_shininess;
            
            // ���ݳ�����Ϣ
            shader.uniforms["u_camera_pos"] = camera.GetPosition();
            shader.uniforms["u_lights"] = scene.GetDirectionalLights(); // ֱ�Ӵ�������vector

            const auto& mesh = *object.mesh;
            for (size_t i = 0; i < mesh.indices.size(); i += 3) {
                //// --- ���� B: ��������Ƿ�Խ�� ---
                //if (mesh.indices[i] >= mesh.vertices.size() ||
                //    mesh.indices[i + 1] >= mesh.vertices.size() ||
                //    mesh.indices[i + 2] >= mesh.vertices.size())
                //{
                //    SDL_Log("Error: mesh '%s' has out-of-bounds indices!", object.name.c_str());
                //    continue; // �������������������
                //}
                // 2. ��ÿ��������ö�����ɫ��
                Varyings v0_out = shader.VertexShader(mesh.vertices[mesh.indices[i]]);
                Varyings v1_out = shader.VertexShader(mesh.vertices[mesh.indices[i + 1]]);
                Varyings v2_out = shader.VertexShader(mesh.vertices[mesh.indices[i + 2]]);

                // TODO: �ü� (Clipping) �����������
                // std::vector<Varyings> clipped_tris = ClipTriangle(...);
                // for (auto& tri : clipped_tris) { ... }
                // --- 2. �ü� ---
                std::vector<Varyings> clipped_triangles = ClipTriangle(v0_out, v1_out, v2_out);

                // 3. �����ü��������������������
                for (size_t j = 0; j < clipped_triangles.size(); j += 3) {
                    Varyings& cv0 = clipped_triangles[j];
                    Varyings& cv1 = clipped_triangles[j + 1];
                    Varyings& cv2 = clipped_triangles[j + 2];

                    // 4. ��ÿ���ü���������ν���͸�ӳ���
                    cv0.position_clip = cv0.position_clip * (1.0f / cv0.position_clip.w());
                    cv1.position_clip = cv1.position_clip * (1.0f / cv1.position_clip.w());
                    cv2.position_clip = cv2.position_clip * (1.0f / cv2.position_clip.w());

                    // 5. ��դ��
                    RasterizeTriangle(cv0, cv1, cv2, shader);
                }
            }
        }
    }
   
}