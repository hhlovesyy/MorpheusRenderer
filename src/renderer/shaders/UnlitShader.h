// src/renderer/shaders/UnlitShader.h (ĞÂÎÄ¼ş)
#pragma once
#include "../IShader.h"

namespace Morpheus::Renderer {

    class UnlitShader : public IShader {
    public:
        Varyings VertexShader(const Vertex& in) override;
        Math::Vector4f FragmentShader(const Varyings& in, const RenderState& renderState) override;
    };
}
