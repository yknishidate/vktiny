#pragma once
#include "vktiny/Vulkan/Buffer.hpp"
#include <glm/glm.hpp>

namespace vkt
{
    using Index = uint32_t;

    struct Vertex
    {
        glm::vec3 pos;
        glm::vec3 normal;
        glm::vec2 uv;
        glm::vec4 color;
        glm::vec4 tangent;
    };

    class Mesh
    {
    public:
        Mesh() = default;

        void initialize(const Context& context,
                        const std::vector<Vertex>& vertices,
                        const std::vector<Index>& indices,
                        vk::BufferUsageFlags usage,
                        vk::MemoryPropertyFlags properties);

        const auto& getVertices() const { return vertices; }
        const auto& getIndices() const { return indices; }
        const auto& getVertexBuffer() const { return vertexBuffer; }
        const auto& getIndexBuffer() const { return indexBuffer; }

    private:
        std::vector<Vertex> vertices;
        std::vector<Index> indices;
        Buffer vertexBuffer;
        Buffer indexBuffer;
    };
}
