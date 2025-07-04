// src/renderer/Clipping.cpp (���ļ�)
#include "Clipping.h"
#include <functional>

namespace Morpheus::Renderer {

    // �������������Բ�ֵ���е�Varyings
    Varyings InterpolateVaryings(const Varyings& v0, const Varyings& v1, float t) {
        Varyings result;
        // ע�⣺����βü��ռ䣬���Ƕ��������Խ��м򵥵����Բ�ֵ
        // position_clip Ҳ�����Բ�ֵ����Ϊ���ǻ�û��͸�ӳ���
        result.position_clip = v0.position_clip * (1.0f - t) + v1.position_clip * t;
        result.color = v0.color * (1.0f - t) + v1.color * t;
        // δ���������ֵ normal, uv, world_pos ��
        return result;
    }

    // ���ĺ�������һ��ƽ��ü�һ�������
    // ʹ�� std::function ����̬����ÿ��ƽ���������Ժ��ཻ����
    std::vector<Varyings> ClipPolygonAgainstPlane(const std::vector<Varyings>& vertices,
        std::function<bool(const Varyings&)> is_inside,
        std::function<Varyings(const Varyings&, const Varyings&)> intersect) {
        std::vector<Varyings> out_vertices;
        if (vertices.empty()) {
            return out_vertices;
        }

        for (size_t i = 0; i < vertices.size(); ++i) {
            const Varyings& current_v = vertices[i];
            const Varyings& prev_v = vertices[(i + vertices.size() - 1) % vertices.size()];

            bool is_current_inside = is_inside(current_v);
            bool is_prev_inside = is_inside(prev_v);

            if (is_current_inside != is_prev_inside) {
                // ����ƽ���ཻ
                out_vertices.push_back(intersect(prev_v, current_v));
            }
            if (is_current_inside) {
                // ��ǰ�������ڲ�
                out_vertices.push_back(current_v);
            }
        }
        return out_vertices;
    }

    // �ܲü�����
    std::vector<Varyings> ClipTriangle(const Varyings& v0, const Varyings& v1, const Varyings& v2) {
        std::vector<Varyings> vertices = { v0, v1, v2 };

        // 1. Vs W plane (Near plane in some conventions, or just w>epsilon)
        vertices = ClipPolygonAgainstPlane(vertices,
            [](const Varyings& v) { return v.position_clip.w() > 0.0001f; }, // Inside if w > epsilon
            [](const Varyings& prev, const Varyings& current) {
                float t = (prev.position_clip.w() - 0.0001f) / (prev.position_clip.w() - current.position_clip.w());
                return InterpolateVaryings(prev, current, t);
            });

        // 2. Vs X planes
        vertices = ClipPolygonAgainstPlane(vertices, [](const Varyings& v) { return v.position_clip.x() <= v.position_clip.w(); }, // Left
            [](const Varyings& prev, const Varyings& current) {
                float t = (prev.position_clip.w() - prev.position_clip.x()) / ((prev.position_clip.w() - prev.position_clip.x()) - (current.position_clip.w() - current.position_clip.x()));
                return InterpolateVaryings(prev, current, t);
            });
        vertices = ClipPolygonAgainstPlane(vertices, [](const Varyings& v) { return v.position_clip.x() >= -v.position_clip.w(); }, // Right
            [](const Varyings& prev, const Varyings& current) {
                float t = (prev.position_clip.w() + prev.position_clip.x()) / ((prev.position_clip.w() + prev.position_clip.x()) - (current.position_clip.w() + current.position_clip.x()));
                return InterpolateVaryings(prev, current, t);
            });

        // 3. Vs Y planes
        vertices = ClipPolygonAgainstPlane(vertices, [](const Varyings& v) { return v.position_clip.y() <= v.position_clip.w(); }, // Top
            [](const Varyings& prev, const Varyings& current) {
                float t = (prev.position_clip.w() - prev.position_clip.y()) / ((prev.position_clip.w() - prev.position_clip.y()) - (current.position_clip.w() - current.position_clip.y()));
                return InterpolateVaryings(prev, current, t);
            });
        vertices = ClipPolygonAgainstPlane(vertices, [](const Varyings& v) { return v.position_clip.y() >= -v.position_clip.w(); }, // Bottom
            [](const Varyings& prev, const Varyings& current) {
                float t = (prev.position_clip.w() + prev.position_clip.y()) / ((prev.position_clip.w() + prev.position_clip.y()) - (current.position_clip.w() + current.position_clip.y()));
                return InterpolateVaryings(prev, current, t);
            });

        // 4. Vs Z planes
        vertices = ClipPolygonAgainstPlane(vertices, [](const Varyings& v) { return v.position_clip.z() <= v.position_clip.w(); }, // Far
            [](const Varyings& prev, const Varyings& current) {
                float t = (prev.position_clip.w() - prev.position_clip.z()) / ((prev.position_clip.w() - prev.position_clip.z()) - (current.position_clip.w() - current.position_clip.z()));
                return InterpolateVaryings(prev, current, t);
            });
        vertices = ClipPolygonAgainstPlane(vertices, [](const Varyings& v) { return v.position_clip.z() >= -v.position_clip.w(); }, // Near
            [](const Varyings& prev, const Varyings& current) {
                float t = (prev.position_clip.w() + prev.position_clip.z()) / ((prev.position_clip.w() + prev.position_clip.z()) - (current.position_clip.w() + current.position_clip.z()));
                return InterpolateVaryings(prev, current, t);
            });


        // 5. Triangulate the final polygon (Fan Triangulation)
        std::vector<Varyings> triangles;
        if (vertices.size() >= 3) {
            for (size_t i = 1; i < vertices.size() - 1; ++i) {
                triangles.push_back(vertices[0]);
                triangles.push_back(vertices[i]);
                triangles.push_back(vertices[i + 1]);
            }
        }

        return triangles;
    }
}
