#include "ShadowMapShader.h"
#include "../Vertex.h"

namespace Morpheus::Renderer {
    Varyings ShadowMapShader::VertexShader(const Vertex& in) {
        Varyings out;
        auto light_space_mvp = std::any_cast<Math::Matrix4f>(uniforms["u_light_space_mvp"]);
        out.position_clip = light_space_mvp * Math::Vector4f{ in.position.x(), in.position.y(), in.position.z(), 1.0f };
        return out;
    }

    Math::Vector4f ShadowMapShader::FragmentShader(const Varyings& in, const RenderState& renderState) {
        // 片元着色器什么都不用做，因为我们只关心深度
        // 返回一个无关紧要的颜色
        return { 0.0f, 0.0f, 0.0f, 1.0f };
    }
}
