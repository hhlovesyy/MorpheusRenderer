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
#include <SDL.h>

namespace Morpheus::Renderer {
    // ... Renderer::Renderer(...) 和 Renderer::DrawLine(...) 函数保持不变 ...

    // --- 构造函数 ---
    Renderer::Renderer(int width, int height) {
        m_framebuffer = std::make_shared<Framebuffer>(width, height);
        m_numThreads = std::thread::hardware_concurrency();
        if (m_numThreads == 0) m_numThreads = 4;
        SDL_Log("Using %u threads for rendering.", m_numThreads);

        const int TILE_SIZE = 64;
        for (int y = 0; y < height; y += TILE_SIZE) {
            for (int x = 0; x < width; x += TILE_SIZE) {
                m_tiles.push_back({
                    x, y,
                    std::min(x + TILE_SIZE, width),
                    std::min(y + TILE_SIZE, height)
                    });
            }
        }
        // 为 m_tilePackets 预留空间
        m_tilePackets.resize(m_tiles.size());
    }

    void Renderer::RasterizeTriangle(const Varyings& v0, const Varyings& v1, const Varyings& v2, IShader& shader, const Tile& tile) {
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

        // 求交集
        int clamped_minX = std::max(minX, tile.minX);
        int clamped_minY = std::max(minY, tile.minY);
        int clamped_maxX = std::min(maxX, tile.maxX);
        int clamped_maxY = std::min(maxY, tile.maxY);

        // 3. 遍历包围盒内的每个像素
        for (int y = clamped_minY; y < clamped_maxY; ++y) {
            for (int x = clamped_minX; x <= clamped_maxX; ++x) {
                // 计算当前像素中心的重心坐标
                Math::vec<2, float> p = { (float)x + 0.5f, (float)y + 0.5f };
                Math::vec<2, float> pa = { p0_screen.x(), p0_screen.y() };
                Math::vec<2, float> pb = { p1_screen.x(), p1_screen.y() };
                Math::vec<2, float> pc = { p2_screen.x(), p2_screen.y() };

                float total_area = (pb.x() - pa.x()) * (pc.y() - pa.y()) - (pc.x() - pa.x()) * (pb.y() - pa.y());
                // 简单的背面剔除，可以放在这里
                if (total_area < 0) continue;

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

                    // 5. 调用片元着色器
                    Math::Vector4f final_color = shader.FragmentShader(interpolated_varyings);

                    // 6. 写入帧缓冲 (深度测试)
                    m_framebuffer->SetPixel(x, y, z_interp, final_color);
                }
            }
        }
    }

    // --- 顶层 Render 函数 (现在更简洁) ---
    void Renderer::Render(const Scene::Scene& scene) {
        // 1. 顶点处理阶段 (单线程)
        SetupFrame(scene);

        // 2. 三角形分配阶段 (单线程)
        DistributePacketsToTiles();

        // 3. 瓦块渲染阶段 (多线程)
        RenderTiles();
    }

    // --- 新增: DistributePacketsToTiles 函数 (核心逻辑) ---
    void Renderer::DistributePacketsToTiles() {
        // 清空上一帧的分配结果
        for (auto& p_list : m_tilePackets) {
            p_list.clear();
        }

        int w = m_framebuffer->GetWidth();
        int h = m_framebuffer->GetHeight();

        // 遍历所有已处理好的渲染包
        for (const auto& packet : m_renderPackets) {
            // 计算 packet 的屏幕包围盒
            float minX_ndc = std::min({ packet.v0.position_clip.x(), packet.v1.position_clip.x(), packet.v2.position_clip.x() });
            float minY_ndc = std::min({ packet.v0.position_clip.y(), packet.v1.position_clip.y(), packet.v2.position_clip.y() });
            float maxX_ndc = std::max({ packet.v0.position_clip.x(), packet.v1.position_clip.x(), packet.v2.position_clip.x() });
            float maxY_ndc = std::max({ packet.v0.position_clip.y(), packet.v1.position_clip.y(), packet.v2.position_clip.y() });

            int tri_min_x = static_cast<int>(std::floor((minX_ndc + 1.0f) * 0.5f * w));
            int tri_min_y = static_cast<int>(std::floor((minY_ndc + 1.0f) * 0.5f * h));
            int tri_max_x = static_cast<int>(std::ceil((maxX_ndc + 1.0f) * 0.5f * w));
            int tri_max_y = static_cast<int>(std::ceil((maxY_ndc + 1.0f) * 0.5f * h));

            // 找到与三角形包围盒相交的瓦块范围
            int start_tile_x = std::max(0, tri_min_x / m_tiles[0].maxX); // 假设 TILE_SIZE 固定
            int end_tile_x = std::min((int)m_tiles.size() / (h / 64) - 1, tri_max_x / m_tiles[0].maxX); // 简化的瓦片索引计算
            // ... (更精确的瓦片索引计算会更复杂，我们先用一个简单遍历)

            // 遍历所有瓦块，判断相交，并将 packet 指针放入对应的列表
            for (size_t i = 0; i < m_tiles.size(); ++i) {
                const auto& tile = m_tiles[i];
                if (!(tri_max_x < tile.minX || tri_min_x >= tile.maxX ||
                    tri_max_y < tile.minY || tri_min_y >= tile.maxY))
                {
                    m_tilePackets[i].push_back(&packet);
                }
            }
        }
    }

    // --- RenderTileTask 函数 (现在极其高效) ---
    void Renderer::RenderTileTask(size_t tile_index) {
        const Tile& tile = m_tiles[tile_index];
        const std::vector<const RenderPacket*>& packets_for_this_tile = m_tilePackets[tile_index];

        // 只遍历分配给这个瓦块的、已经筛选过的三角形列表
        for (const RenderPacket* packet_ptr : packets_for_this_tile) {
            // 直接调用光栅化，不再需要任何粗粒度剔除
            RasterizeTriangle(packet_ptr->v0, packet_ptr->v1, packet_ptr->v2, *packet_ptr->shader, tile);
        }
    }

    // --- RenderTiles 函数 (现在只负责分发 tile_index) ---
    void Renderer::RenderTiles() {
        m_threads.clear();
        for (size_t i = 0; i < m_numThreads; ++i) {
            m_threads.emplace_back([this, i]() {
                for (size_t tile_idx = i; tile_idx < m_tiles.size(); tile_idx += m_numThreads) {
                    RenderTileTask(tile_idx);
                }
                });
        }
        for (auto& t : m_threads) {
            if (t.joinable()) t.join();
        }
    }

    // --- Render 函数的完整重构 ---
    void Renderer::SetupFrame(const Scene::Scene& scene) {
        m_framebuffer->ClearColor({ 0.1f, 0.1f, 0.1f, 1.0f });
        m_framebuffer->ClearDepth(1.0f);

        m_renderPackets.clear(); // 清空上一帧的渲染包

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

            // 1. 设置 Shader Uniforms
            const Math::Matrix4f& modelMatrix = object.transform;
            shader.uniforms["u_model"] = modelMatrix;
            shader.uniforms["u_view"] = viewMatrix;
            shader.uniforms["u_projection"] = projectionMatrix;
            shader.uniforms["u_mvp"] = projectionMatrix * viewMatrix * modelMatrix;

            // --- 新增的 Uniforms ---
            // 计算并传递法线矩阵
            shader.uniforms["u_normal_matrix"] = modelMatrix.inverse().transpose();

            // 传递材质参数
            shader.uniforms["u_albedo_factor"] = material.albedo_factor;
			shader.uniforms["u_albedo_texture"] = material.albedo_texture;
			shader.uniforms["u_normal_texture"] = material.normal_texture;
            shader.uniforms["u_shininess"] = material.specular_shininess;
            
            // 传递场景信息
            shader.uniforms["u_camera_pos"] = camera.GetPosition();
            shader.uniforms["u_lights"] = scene.GetDirectionalLights(); // 直接传递整个vector

            const auto& mesh = *object.mesh;
            for (size_t i = 0; i < mesh.indices.size(); i += 3) {
                // 2. 对每个顶点调用顶点着色器
                Varyings v0_out = shader.VertexShader(mesh.vertices[mesh.indices[i]]);
                Varyings v1_out = shader.VertexShader(mesh.vertices[mesh.indices[i + 1]]);
                Varyings v2_out = shader.VertexShader(mesh.vertices[mesh.indices[i + 2]]);

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

                    // --- 创建并存储渲染包 ---
                    // 我们将处理好的顶点和指向当前 shader 的指针打包在一起
                    m_renderPackets.push_back({ cv0, cv1, cv2, &shader });
                }
                
            }
        }
    }
   
}