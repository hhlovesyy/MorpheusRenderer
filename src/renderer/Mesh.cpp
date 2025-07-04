#include "Mesh.h"
#include <stdexcept>

// 在这里，且只在这里，定义实现宏
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
            // 最好打印出来，方便调试
            if (!warn.empty()) {
                // std::cout << "TinyObjLoader Warning: " << warn << std::endl;
            }
            if (!err.empty()) {
                throw std::runtime_error("TinyObjLoader Error: " + err);
            }
            throw std::runtime_error("Failed to load OBJ file for unknown reasons.");
        }

        // 遍历所有顶点数据
        for (const auto& shape : shapes) {
            for (const auto& index : shape.mesh.indices) {
                Vertex vertex{};
                vertex.position = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]
                };

                // 暂时不处理法线和UV，但可以为以后预留位置
                // if (index.normal_index >= 0) { ... }
                // if (index.texcoord_index >= 0) { ... }

                mesh.vertices.push_back(vertex);
                // 注意：OBJ文件通常每个面有独立的顶点，所以我们直接用线性索引
                mesh.indices.push_back(mesh.indices.size());
            }
        }
        return mesh;
    }

}
