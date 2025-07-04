#pragma once
#include <memory>
#include "Framebuffer.h"
#include "Mesh.h"
#include "../scene/Scene.h"
#include "../math/Matrix.h"

namespace Morpheus::Renderer {
    class Renderer {
    public:
        // --- 新增这个结构体 ---
        // 保存顶点着色器输出，并由光栅化器使用的顶点
        struct ShadedVertex {
            Math::Vector4f position_clip; // 裁剪空间坐标 (x,y,z,w)
            // 未来要插值的属性放这里
            // Math::Vector3f position_world;
            // Math::Vector3f normal_world;
            // Math::Vector2f tex_coords;
            Math::Vector4f color; // 暂时只插值颜色
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
