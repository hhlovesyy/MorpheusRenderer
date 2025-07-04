// src/renderer/Mesh.h - 正确的版本
#pragma once
#include "../math/Vector.h"
#include <vector>
#include <string>
#include "Vertex.h"

// 只包含头文件，不进行实现！
// 我们甚至不需要在这里包含 tiny_obj_loader.h，因为实现细节被隐藏在了 .cpp 文件中
// 这是一种更好的封装实践，可以减少头文件依赖和编译时间

namespace Morpheus::Renderer {
    class Mesh {
    public:
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

        // 这里只有函数的声明，以分号结尾
        static Mesh LoadFromObj(const std::string& filepath);
    };
}
