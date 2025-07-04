#pragma once
#include <memory>
#include "Framebuffer.h"
#include "Mesh.h"
#include "../scene/Scene.h"
#include "../math/Matrix.h"

namespace Morpheus::Renderer {
    class Renderer {
    public:
        // --- ��������ṹ�� ---
        // ���涥����ɫ����������ɹ�դ����ʹ�õĶ���
        struct ShadedVertex {
            Math::Vector4f position_clip; // �ü��ռ����� (x,y,z,w)
            // δ��Ҫ��ֵ�����Է�����
            // Math::Vector3f position_world;
            // Math::Vector3f normal_world;
            // Math::Vector2f tex_coords;
            Math::Vector4f color; // ��ʱֻ��ֵ��ɫ
        };
        Renderer(int width, int height);
        //void DrawLine(int x0, int y0, int x1, int y1, const Math::Vector4f& color);
        void Render(const Scene::Scene& scene);
        void RasterizeTriangle(const ShadedVertex& v0, const ShadedVertex& v1, const ShadedVertex& v2);
        std::shared_ptr<Framebuffer> GetFramebuffer() const { return m_framebuffer; }
    private:
        std::shared_ptr<Framebuffer> m_framebuffer;
    };
}
