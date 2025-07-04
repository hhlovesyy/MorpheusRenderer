#include "Mesh.h"
#include <stdexcept>

// �������ֻ���������ʵ�ֺ�
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

namespace Morpheus::Renderer {

    Mesh Mesh::LoadFromObj(const std::string& filepath) {
        Mesh mesh;
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filepath.c_str())) {
            // ��ô�ӡ�������������
            if (!warn.empty()) {
                // std::cout << "TinyObjLoader Warning: " << warn << std::endl;
            }
            if (!err.empty()) {
                throw std::runtime_error("TinyObjLoader Error: " + err);
            }
            throw std::runtime_error("Failed to load OBJ file for unknown reasons.");
        }

        // �������ж�������
        for (const auto& shape : shapes) {
            for (const auto& index : shape.mesh.indices) {
                Vertex vertex{};
                vertex.position = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]
                };

                // ��ʱ�������ߺ�UV��������Ϊ�Ժ�Ԥ��λ��
                // if (index.normal_index >= 0) { ... }
                // if (index.texcoord_index >= 0) { ... }

                mesh.vertices.push_back(vertex);
                // ע�⣺OBJ�ļ�ͨ��ÿ�����ж����Ķ��㣬��������ֱ������������
                mesh.indices.push_back(mesh.indices.size());
            }
        }
        return mesh;
    }

}
