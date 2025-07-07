// src/renderer/RenderState.h (���߰����� src/renderer/Renderer.h ��)
#pragma once

#include "../math/Vector.h"
#include "../math/Matrix.h"
#include <cstdint> // For uint32_t

// Forward declarations (��� Framebuffer �� Texture ��������ͷ�ļ��ж����)
namespace Morpheus::Renderer {
    class Framebuffer;
    class Texture;
}

namespace Morpheus::Renderer {

    // --- ��Ⱦ״̬��־ ---
    // ʹ�� enum class �ṩ���Ͱ�ȫ�͸��õĿɶ���
    enum class RenderStateFlags {
        None = 0,
        DepthWriteEnable = 1 << 0,  // �Ƿ�д����Ȼ���
        DepthTestEnable = 1 << 1,  // �Ƿ������Ȳ���
        CullFaceEnable = 1 << 2,  // �Ƿ����ñ����޳�
        BlendEnable = 1 << 3,  // �Ƿ����� Alpha ���
        // ������Ӹ����־�����磺StencilTestEnable, WireframeEnable ��
    };

    // --- RenderState �ṹ�� ---
    // ����ṹ�����Ϊ�������ݸ����� RenderPass �� Shader
    struct RenderState {
        // --- ��ȾĿ�� ---
        Framebuffer* targetFramebuffer = nullptr; // ��ǰ��Ⱦ������Ŀ�� Framebuffer

        // --- Shadow Map �����Ϣ ---
        // ָ�� Shadow Map ���������
        // ע�⣺�ڳ�ʼ�׶Σ����ǿ���ֱ�Ӵ����������ָ��� Shader
        // �Ժ���ܻ��и����Ƶ�����Ԫ������
        const Texture* shadowMapTexture = nullptr;
        // ��Դ�� View-Projection �������ڽ���������ת������Դ�Ĳü��ռ�
        Math::Matrix4f lightViewProjectionMatrix;

        // --- ��Ⱦģʽ��־ ---
        // ���������� RenderStateFlags
        // ʹ�������������洢��־λ���������λ����
        uint32_t flags = static_cast<uint32_t>(RenderStateFlags::None);

        // --- �����������������úͼ���־λ ---

        // ���һ��������־
        void AddFlags(RenderStateFlags newFlags) {
            flags |= static_cast<uint32_t>(newFlags);
        }

        // �Ƴ�һ����־
        void RemoveFlags(RenderStateFlags flagsToRemove) {
            flags &= ~static_cast<uint32_t>(flagsToRemove);
        }

        // ����Ƿ�������ĳ����־
        bool IsFlagEnabled(RenderStateFlags flag) const {
            return (flags & static_cast<uint32_t>(flag)) != 0;
        }

        // --- ���캯�� (��ѡ���������ʼ��) ---
        RenderState() = default; // ʹ��Ĭ�Ϲ��캯����״̬�����Ժ�����
    };

} // namespace Morpheus::Renderer
