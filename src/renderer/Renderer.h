#pragma once
#include <memory>
#include "Framebuffer.h"
#include "Mesh.h"
#include "../scene/Scene.h"
#include "../math/Matrix.h"

// ǰ������
namespace Morpheus::Scene { class Scene; }
namespace Morpheus::Renderer { struct Varyings; class IShader; }

namespace Morpheus::Renderer {
    class Renderer {
    public:
        Renderer(int width, int height);
        void Render(const Scene::Scene& scene);
        std::shared_ptr<Framebuffer> GetFramebuffer() const { return m_framebuffer; }

    private:
        // --- �����޸ģ�RasterizeTriangle ������Ҫһ�� shader ---
        void RasterizeTriangle(const Varyings& v0, const Varyings& v1, const Varyings& v2, IShader& shader);

        std::shared_ptr<Framebuffer> m_framebuffer;
    };
}
