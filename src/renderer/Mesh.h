// src/renderer/Mesh.h - ��ȷ�İ汾
#pragma once
#include "../math/Vector.h"
#include <vector>
#include <string>
#include "Vertex.h"

// ֻ����ͷ�ļ���������ʵ�֣�
// ������������Ҫ��������� tiny_obj_loader.h����Ϊʵ��ϸ�ڱ��������� .cpp �ļ���
// ����һ�ָ��õķ�װʵ�������Լ���ͷ�ļ������ͱ���ʱ��

namespace Morpheus::Renderer {
    class Mesh {
    public:
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

        // ����ֻ�к������������ԷֺŽ�β
        static Mesh LoadFromObj(const std::string& filepath);
    };
}
