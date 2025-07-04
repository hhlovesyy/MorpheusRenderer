// src/renderer/Renderer.cpp
#include "Renderer.h"
#include "../math/Common.h" // <--- 包含新的头文件
#include <algorithm>
#include <vector>
#include <cmath>
#include <iostream>
#include "../scene/Scene.h"

namespace Morpheus::Renderer {
    // ... Renderer::Renderer(...) 和 Renderer::DrawLine(...) 函数保持不变 ...

    Renderer::Renderer(int width, int height) {
        m_framebuffer = std::make_shared<Framebuffer>(width, height);
    }

    void Renderer::RasterizeTriangle(const ShadedVertex& v0, const ShadedVertex& v1, const ShadedVertex& v2) {
        // 1. 视口变换
        int w = m_framebuffer->GetWidth();
        int h = m_framebuffer->GetHeight();
        Math::vec<3, float> p0 = { (v0.position_clip.x() + 1.0f) * 0.5f * w, (v0.position_clip.y() + 1.0f) * 0.5f * h, v0.position_clip.z() };
        Math::vec<3, float> p1 = { (v1.position_clip.x() + 1.0f) * 0.5f * w, (v1.position_clip.y() + 1.0f) * 0.5f * h, v1.position_clip.z() };
        Math::vec<3, float> p2 = { (v2.position_clip.x() + 1.0f) * 0.5f * w, (v2.position_clip.y() + 1.0f) * 0.5f * h, v2.position_clip.z() };

        // 2. 计算包围盒 (Bounding Box)
        int minX = std::max(0, (int)std::floor(std::min({ p0.x(), p1.x(), p2.x() })));
        int minY = std::max(0, (int)std::floor(std::min({ p0.y(), p1.y(), p2.y() })));
        int maxX = std::min(w - 1, (int)std::ceil(std::max({ p0.x(), p1.x(), p2.x() })));
        int maxY = std::min(h - 1, (int)std::ceil(std::max({ p0.y(), p1.y(), p2.y() })));

        // 3. 遍历包围盒内的每个像素
        for (int y = minY; y <= maxY; ++y) {
            for (int x = minX; x <= maxX; ++x) {
                // 计算当前像素中心的重心坐标
                Math::vec<2, float> p = { (float)x + 0.5f, (float)y + 0.5f };
                Math::vec<2, float> pa = { p0.x(), p0.y() };
                Math::vec<2, float> pb = { p1.x(), p1.y() };
                Math::vec<2, float> pc = { p2.x(), p2.y() };

                float total_area = (pb.x() - pa.x()) * (pc.y() - pa.y()) - (pc.x() - pa.x()) * (pb.y() - pa.y());
                float w0 = ((pb.x() - p.x()) * (pc.y() - p.y()) - (pc.x() - p.x()) * (pb.y() - p.y())) / total_area;
                float w1 = ((pc.x() - p.x()) * (pa.y() - p.y()) - (pa.x() - p.x()) * (pc.y() - p.y())) / total_area;
                float w2 = 1.0f - w0 - w1;

                // 如果重心坐标都在[0,1]范围内，则像素在三角形内
                if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
                    // 4. 透视校正插值
                    // 顶点属性需要除以w，插值后再乘以w
                    float one_over_w0 = 1.0f / v0.position_clip.w();
                    float one_over_w1 = 1.0f / v1.position_clip.w();
                    float one_over_w2 = 1.0f / v2.position_clip.w();

                    // 插值 1/w
                    float one_over_w_interp = w0 * one_over_w0 + w1 * one_over_w1 + w2 * one_over_w2;
                    float w_interp = 1.0f / one_over_w_interp;

                    // 插值颜色 (color/w)
                    Math::Vector4f color_interp = (v0.color * (one_over_w0 * w0) +
                        v1.color * (one_over_w1 * w1) +
                        v2.color * (one_over_w2 * w2)) * w_interp;

                   
                    // --- 新增：插值深度值 ---
                    // 注意：屏幕空间的Z值(NDC)也是需要进行透视校正插值的！
                    // 这非常重要，但初学者经常会忘记。
                    float z0_ndc = v0.position_clip.z();
                    float z1_ndc = v1.position_clip.z();
                    float z2_ndc = v2.position_clip.z();

                    // 和颜色一样，对 z/w 进行插值
                    float z_over_w_interp = (z0_ndc * one_over_w0) * w0 +
                        (z1_ndc * one_over_w1) * w1 +
                        (z2_ndc * one_over_w2) * w2;

                    float z_interp = z_over_w_interp * w_interp;

                    // 5. Fragment Shader & 写入帧缓冲
                    // --- 将插值后的深度值 z_interp 传递给 SetPixel ---
                    m_framebuffer->SetPixel(x, y, z_interp, color_interp);
                }
            }
        }
    }

    void Renderer::Render(const Scene::Scene& scene) {
        m_framebuffer->ClearColor({ 0.1f, 0.1f, 0.1f, 1.0f });
        m_framebuffer->ClearDepth(1.0f); // <--- 新增：每帧开始时将深度设为最远
        // 从场景中获取相机和其矩阵
        const auto& camera = scene.GetCamera();
        const Math::Matrix4f& viewMatrix = camera.GetViewMatrix();
        const Math::Matrix4f& projectionMatrix = camera.GetProjectionMatrix();

        // 遍历场景中的所有对象并渲染它们
        for (const auto& object : scene.GetObjects())
        {

            if (!object.mesh || !object.material) continue;

            // 组合 MVP 矩阵
            const Math::Matrix4f& modelMatrix = object.transform;
            Math::Matrix4f mvp = projectionMatrix * viewMatrix * modelMatrix;
            // 遍历对象的网格中的所有三角形
            for (size_t i = 0; i < object.mesh->indices.size(); i += 3) {
                // --- 1. Vertex Shader ---
                ShadedVertex v0_out, v1_out, v2_out;

                // --- Process V0 ---
                Vertex v0_in = object.mesh->vertices[object.mesh->indices[i]];
                v0_out.position_clip = mvp * Math::Vector4f{ v0_in.position.x(), v0_in.position.y(), v0_in.position.z(), 1.0f };
                // 从材质获取颜色
                v0_out.color = object.material->color;

                // --- Process V1 ---
                Vertex v1_in = object.mesh->vertices[object.mesh->indices[i + 1]];
                v1_out.position_clip = mvp * Math::Vector4f{ v1_in.position.x(), v1_in.position.y(), v1_in.position.z(), 1.0f};
                v1_out.color = object.material->color;

                // --- Process V2 ---
                Vertex v2_in = object.mesh->vertices[object.mesh->indices[i + 2]];
                v2_out.position_clip = mvp * Math::Vector4f{ v2_in.position.x(), v2_in.position.y(), v2_in.position.z(), 1.0f };
                v2_out.color = object.material->color;

                // TODO: Clipping will go here

                // --- 2. Perspective Divide ---
                // (这段逻辑可以提取成一个辅助函数)
                v0_out.position_clip = v0_out.position_clip * (1.0f / v0_out.position_clip.w());
                v1_out.position_clip = v1_out.position_clip * (1.0f / v1_out.position_clip.w());
                v2_out.position_clip = v2_out.position_clip * (1.0f / v2_out.position_clip.w());

                // --- 3. Rasterization ---
                RasterizeTriangle(v0_out, v1_out, v2_out);
            }
        }
    }
   
}