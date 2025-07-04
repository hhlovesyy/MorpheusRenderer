// src/renderer/Renderer.cpp
#include "Renderer.h"
#include "../math/Common.h" // <--- 包含新的头文件
#include <algorithm>
#include <vector>
#include <cmath>
#include <iostream>
#include "../scene/Scene.h"
#include "IShader.h"
#include "../renderer/Clipping.h"

namespace Morpheus::Renderer {
    // ... Renderer::Renderer(...) 和 Renderer::DrawLine(...) 函数保持不变 ...

    Renderer::Renderer(int width, int height) {
        m_framebuffer = std::make_shared<Framebuffer>(width, height);
    }

    void Renderer::RasterizeTriangle(const Varyings& v0, const Varyings& v1, const Varyings& v2, IShader& shader) {
        // 1. 视口变换 (与之前相同，但操作Varyings)
        int w = m_framebuffer->GetWidth();
        int h = m_framebuffer->GetHeight();
        Math::vec<3, float> p0_screen = { (v0.position_clip.x() + 1.0f) * 0.5f * w, (v0.position_clip.y() + 1.0f) * 0.5f * h, v0.position_clip.z() };
        Math::vec<3, float> p1_screen = { (v1.position_clip.x() + 1.0f) * 0.5f * w, (v1.position_clip.y() + 1.0f) * 0.5f * h, v1.position_clip.z() };
        Math::vec<3, float> p2_screen = { (v2.position_clip.x() + 1.0f) * 0.5f * w, (v2.position_clip.y() + 1.0f) * 0.5f * h, v2.position_clip.z() };

        // 2. 计算包围盒 (与之前相同)
        int minX = std::max(0, (int)std::floor(std::min({ p0_screen.x(), p1_screen.x(), p2_screen.x() })));
        int minY = std::max(0, (int)std::floor(std::min({ p0_screen.y(), p1_screen.y(), p2_screen.y() })));
        int maxX = std::min(w - 1, (int)std::ceil(std::max({ p0_screen.x(), p1_screen.x(), p2_screen.x() })));
        int maxY = std::min(h - 1, (int)std::ceil(std::max({ p0_screen.y(), p1_screen.y(), p2_screen.y() })));

        // 3. 遍历包围盒内的每个像素
        for (int y = minY; y <= maxY; ++y) {
            for (int x = minX; x <= maxX; ++x) {
                // 计算当前像素中心的重心坐标
                Math::vec<2, float> p = { (float)x + 0.5f, (float)y + 0.5f };
                Math::vec<2, float> pa = { p0_screen.x(), p0_screen.y() };
                Math::vec<2, float> pb = { p1_screen.x(), p1_screen.y() };
                Math::vec<2, float> pc = { p2_screen.x(), p2_screen.y() };

                float total_area = (pb.x() - pa.x()) * (pc.y() - pa.y()) - (pc.x() - pa.x()) * (pb.y() - pa.y());
                float w0 = ((pb.x() - p.x()) * (pc.y() - p.y()) - (pc.x() - p.x()) * (pb.y() - p.y())) / total_area;
                float w1 = ((pc.x() - p.x()) * (pa.y() - p.y()) - (pa.x() - p.x()) * (pc.y() - p.y())) / total_area;
                float w2 = 1.0f - w0 - w1;

                // 如果重心坐标都在[0,1]范围内，则像素在三角形内
                if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
                    float one_over_w0 = 1.0f / v0.position_clip.w();
                    float one_over_w1 = 1.0f / v1.position_clip.w();
                    float one_over_w2 = 1.0f / v2.position_clip.w();
                    float one_over_w_interp = w0 * one_over_w0 + w1 * one_over_w1 + w2 * one_over_w2;
                    float w_interp = 1.0f / one_over_w_interp;

                    // 插值深度
                    float z_interp = (p0_screen.z() * w0 + p1_screen.z() * w1 + p2_screen.z() * w2);

                    // --- 核心修改：插值所有 Varyings ---
                    Varyings interpolated_varyings;
                    // 对 color/w 进行插值
                    interpolated_varyings.color = ((v0.color * one_over_w0) * w0 +
                        (v1.color * one_over_w1) * w1 +
                        (v2.color * one_over_w2) * w2) * w_interp;

                    // 5. 调用片元着色器
                    Math::Vector4f final_color = shader.FragmentShader(interpolated_varyings);

                    // 6. 写入帧缓冲 (深度测试)
                    m_framebuffer->SetPixel(x, y, z_interp, final_color);
                }
            }
        }
    }

    // --- Render 函数的完整重构 ---
    void Renderer::Render(const Scene::Scene& scene) {
        m_framebuffer->ClearColor({ 0.1f, 0.1f, 0.1f, 1.0f });
        m_framebuffer->ClearDepth(1.0f);

        const auto& camera = scene.GetCamera();
        const Math::Matrix4f& viewMatrix = camera.GetViewMatrix();
        const Math::Matrix4f& projectionMatrix = camera.GetProjectionMatrix();

        for (const auto& object : scene.GetObjects()) {
            if (!object.mesh || !object.material || !object.material->shader) {
                continue;
            }

            auto& shader = *object.material->shader;

            // 1. 设置 Shader Uniforms
            shader.uniforms["u_mvp"] = projectionMatrix * viewMatrix * object.transform;
            shader.uniforms["u_model"] = object.transform;
            shader.uniforms["u_view"] = viewMatrix;
            shader.uniforms["u_projection"] = projectionMatrix;
            shader.uniforms["u_color"] = object.material->color;
            // 未来可以传递更多uniforms, e.g., camera position, lights...

            const auto& mesh = *object.mesh;
            for (size_t i = 0; i < mesh.indices.size(); i += 3) {
                // 2. 对每个顶点调用顶点着色器
                Varyings v0_out = shader.VertexShader(mesh.vertices[mesh.indices[i]]);
                Varyings v1_out = shader.VertexShader(mesh.vertices[mesh.indices[i + 1]]);
                Varyings v2_out = shader.VertexShader(mesh.vertices[mesh.indices[i + 2]]);

                // TODO: 裁剪 (Clipping) 将在这里进行
                // std::vector<Varyings> clipped_tris = ClipTriangle(...);
                // for (auto& tri : clipped_tris) { ... }
                // --- 2. 裁剪 ---
                std::vector<Varyings> clipped_triangles = ClipTriangle(v0_out, v1_out, v2_out);

                // 3. 遍历裁剪后产生的所有新三角形
                for (size_t j = 0; j < clipped_triangles.size(); j += 3) {
                    Varyings& cv0 = clipped_triangles[j];
                    Varyings& cv1 = clipped_triangles[j + 1];
                    Varyings& cv2 = clipped_triangles[j + 2];

                    // 4. 对每个裁剪后的三角形进行透视除法
                    cv0.position_clip = cv0.position_clip * (1.0f / cv0.position_clip.w());
                    cv1.position_clip = cv1.position_clip * (1.0f / cv1.position_clip.w());
                    cv2.position_clip = cv2.position_clip * (1.0f / cv2.position_clip.w());

                    // 5. 光栅化
                    RasterizeTriangle(cv0, cv1, cv2, shader);
                }
            }
        }
    }
   
}