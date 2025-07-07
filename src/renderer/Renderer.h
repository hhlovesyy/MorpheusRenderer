#pragma once
#include <memory>
#include "Framebuffer.h"
#include "Mesh.h"
#include "../scene/Scene.h"
#include "../math/Matrix.h"
#include "Material.h"
#include <thread>

// ǰ������
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
    // ����ṹ�������Ⱦһ�������������������Ϣ
    struct RenderPacket {
        Varyings v0, v1, v2;
        IShader* shader; // ָ���������Ӧʹ�õ� Shader ʵ��
        RenderState renderState;
    };
    class Renderer {
    public:
        Renderer(int width, int height);
        void Render(const Scene::Scene& scene);
        std::shared_ptr<Framebuffer> GetFramebuffer() const { return m_framebuffer; }

    private:
        void SetupFrame(const Scene::Scene& scene); // ׼���׶Σ��������ж���
        void RenderTiles(); // ��Ⱦ�׶Σ��������߳���Ⱦ
        // --- �����޸ģ�RasterizeTriangle ������Ҫһ�� shader ---
        void RasterizeTriangle(const Varyings& v0, const Varyings& v1, const Varyings& v2, IShader& shader, const Tile& tile, const RenderState& renderState);

        std::shared_ptr<Framebuffer> m_framebuffer;
        // �洢���о��� VS���ü���͸�ӳ�����������ζ���
        std::vector<RenderPacket> m_renderPackets;    // <-- ����һ���滻
        // --- ���������ĺ������ݽṹ ---
        // ���������� m_tiles ������һһ��Ӧ��
        // ÿ��Ԫ����һ�� vector���洢��ָ�� m_renderPackets ��Ԫ�ص�ָ�롣
        std::vector<std::vector<const RenderPacket*>> m_tilePackets;

        // --- �����ĺ������� ---
        void DistributePacketsToTiles();

        // --- �޸� RenderTileTask ��ǩ�� ---
        // �����ڽ���һ�� tile_index������������ Tile ����
        void RenderTileTask(size_t tile_index);

        // --- �̹߳��� ---
        unsigned int m_numThreads;
        std::vector<std::thread> m_threads;
        std::vector<Tile> m_tiles;

        void ProcessRenderQueue(const std::vector<RenderCommand>& queue, const Scene::Scene& scene, bool is_transparent_pass);

        // --- ������Ⱦ���� ---
        std::vector<RenderCommand> m_renderQueues[static_cast<size_t>(RenderQueue::Count)];
    };
}
