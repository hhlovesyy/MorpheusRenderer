#pragma once
#include <memory>
#include "Framebuffer.h"
#include "Mesh.h"
#include "../scene/Scene.h"
#include "../math/Matrix.h"
#include "Material.h"
#include <thread>

// 前向声明
namespace Morpheus::Scene { class Scene; }
namespace Morpheus::Renderer { struct Varyings; class IShader; }

namespace Morpheus::Renderer {
    struct RenderCommand {
        const Scene::SceneObject* object;
        float distance_to_camera_sq;
    };
    struct Tile {
        int minX, minY;
        int maxX, maxY;
    };
    // 这个结构体包含渲染一个三角形所需的所有信息
    struct RenderPacket {
        Varyings v0, v1, v2;
        IShader* shader; // 指向该三角形应使用的 Shader 实例
        RenderState renderState;
    };
    class Renderer {
    public:
        Renderer(int width, int height);
        void Render(const Scene::Scene& scene);
        std::shared_ptr<Framebuffer> GetFramebuffer() const { return m_framebuffer; }

    private:
        void SetupFrame(const Scene::Scene& scene); // 准备阶段：处理所有顶点
        void RenderTiles(); // 渲染阶段：启动多线程渲染
        // --- 核心修改：RasterizeTriangle 现在需要一个 shader ---
        void RasterizeTriangle(const Varyings& v0, const Varyings& v1, const Varyings& v2, IShader& shader, const Tile& tile, const RenderState& renderState);

        std::shared_ptr<Framebuffer> m_framebuffer;
        // 存储所有经过 VS、裁剪、透视除法后的三角形顶点
        std::vector<RenderPacket> m_renderPackets;    // <-- 用这一行替换
        // --- 这是新增的核心数据结构 ---
        // 它的索引与 m_tiles 的索引一一对应。
        // 每个元素是一个 vector，存储了指向 m_renderPackets 中元素的指针。
        std::vector<std::vector<const RenderPacket*>> m_tilePackets;

        // --- 新增的函数声明 ---
        void DistributePacketsToTiles();

        // --- 修改 RenderTileTask 的签名 ---
        // 它现在接收一个 tile_index，而不是整个 Tile 对象
        void RenderTileTask(size_t tile_index);

        // --- 线程管理 ---
        unsigned int m_numThreads;
        std::vector<std::thread> m_threads;
        std::vector<Tile> m_tiles;

        void ProcessRenderQueue(const std::vector<RenderCommand>& queue, const Scene::Scene& scene, bool is_transparent_pass);

        // --- 新增渲染队列 ---
        std::vector<RenderCommand> m_renderQueues[static_cast<size_t>(RenderQueue::Count)];
    };
}
