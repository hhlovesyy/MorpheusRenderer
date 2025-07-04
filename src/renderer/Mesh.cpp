#include "Mesh.h"
#include <stdexcept>
#include <SDL.h>

// �������ֻ���������ʵ�ֺ�
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

namespace Morpheus::Renderer {
    void CalculateTangents(std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) {
        if (vertices.empty() || indices.empty()) return;

        // ����һ����ʱ�������ۼ�ÿ����������ߺ͸�����
        std::vector<Math::Vector3f> temp_tangents(vertices.size(), { 0, 0, 0 });
        std::vector<Math::Vector3f> temp_bitangents(vertices.size(), { 0, 0, 0 });

        for (size_t i = 0; i < indices.size(); i += 3) {
            Vertex& v0 = vertices[indices[i]];
            Vertex& v1 = vertices[indices[i + 1]];
            Vertex& v2 = vertices[indices[i + 2]];

            // ����ߺ�UV��
            Math::Vector3f edge1 = v1.position - v0.position;
            Math::Vector3f edge2 = v2.position - v0.position;
            Math::Vector2f deltaUV1 = v1.texCoords - v0.texCoords;
            Math::Vector2f deltaUV2 = v2.texCoords - v0.texCoords;

            float f = 1.0f / (deltaUV1.x() * deltaUV2.y() - deltaUV2.x() * deltaUV1.y());
            if (std::isinf(f) || std::isnan(f)) f = 0.0f;

            Math::Vector3f tangent;
            tangent.x() = f * (deltaUV2.y() * edge1.x() - deltaUV1.y() * edge2.x());
            tangent.y() = f * (deltaUV2.y() * edge1.y() - deltaUV1.y() * edge2.y());
            tangent.z() = f * (deltaUV2.y() * edge1.z() - deltaUV1.y() * edge2.z());

            Math::Vector3f bitangent;
            bitangent.x() = f * (-deltaUV2.x() * edge1.x() + deltaUV1.x() * edge2.x());
            bitangent.y() = f * (-deltaUV2.x() * edge1.y() + deltaUV1.x() * edge2.y());
            bitangent.z() = f * (-deltaUV2.x() * edge1.z() + deltaUV1.x() * edge2.z());

            // �ۼӵ�����������
            temp_tangents[indices[i]] = temp_tangents[indices[i]] + tangent;
            temp_tangents[indices[i + 1]] = temp_tangents[indices[i + 1]] + tangent;
            temp_tangents[indices[i + 2]] = temp_tangents[indices[i + 2]] + tangent;

            temp_bitangents[indices[i]] = temp_bitangents[indices[i]] + bitangent;
            temp_bitangents[indices[i + 1]] = temp_bitangents[indices[i + 1]] + bitangent;
            temp_bitangents[indices[i + 2]] = temp_bitangents[indices[i + 2]] + bitangent;
        }

        // ��ÿ��������� Gram-Schmidt �������͹�һ��
        for (size_t i = 0; i < vertices.size(); ++i) {
            const Math::Vector3f& n = vertices[i].normal;
            const Math::Vector3f& t = temp_tangents[i];

            // ������
            vertices[i].tangent = Math::normalize(t - n * Math::dot(n, t));

            // ���㸱���ߵ����� (handedness)
            if (Math::dot(Math::cross(n, vertices[i].tangent), temp_bitangents[i]) < 0.0f) {
                vertices[i].tangent = vertices[i].tangent * -1.0f;
            }
        }
    }

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
        if (attrib.normals.empty()) {
            SDL_Log("Warning: Mesh '%s' has no normals in file!", filepath.c_str());
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

                // --- �������ط��ߵĴ��� ---
                if (index.normal_index >= 0) {
                    vertex.normal = {
                        attrib.normals[3 * index.normal_index + 0],
                        attrib.normals[3 * index.normal_index + 1],
                        attrib.normals[3 * index.normal_index + 2]
                    };
                }

                if (index.texcoord_index >= 0) {
                    vertex.texCoords = {
                        attrib.texcoords[2 * index.texcoord_index + 0],
                        attrib.texcoords[2 * index.texcoord_index + 1]
                    };
                }

                mesh.vertices.push_back(vertex);
                // ע�⣺OBJ�ļ�ͨ��ÿ�����ж����Ķ��㣬��������ֱ������������
                mesh.indices.push_back(mesh.indices.size());
            }
        }
        CalculateTangents(mesh.vertices, mesh.indices);
        SDL_Log("Calculated tangents for mesh: %s", filepath.c_str());
        return mesh;
    }

}
