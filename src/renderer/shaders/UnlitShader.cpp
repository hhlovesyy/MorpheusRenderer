#include "UnlitShader.h"
#include "../Vertex.h"

namespace Morpheus::Renderer {
    Varyings UnlitShader::VertexShader(const Vertex& in) {
        Varyings out;
        auto mvp = std::any_cast<Math::Matrix4f>(uniforms["u_mvp"]);
        out.position_clip = mvp * Math::Vector4f{ in.position.x(), in.position.y(), in.position.z(), 1.0f };

        auto color = std::any_cast<Math::Vector4f>(uniforms["u_albedo_factor"]);
        out.color = { color.x(), color.y(), color.z() };
        return out;
    }

    Math::Vector4f UnlitShader::FragmentShader(const Varyings& in) {
        return { in.color.x(), in.color.y(), in.color.z(), 1.0f };
    }
}
