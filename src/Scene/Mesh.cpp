#include "vktiny/Scene/Mesh.hpp"

void vkt::Mesh::initialize(const Context& context, const std::vector<Vertex>& vertices, const std::vector<Index>& indices, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties)
{
    this->vertices = vertices;
    this->indices = indices;
    vertexBuffer.initialize(context,
                            sizeof(vkt::Vertex) * vertices.size(),
                            usage, properties, this->vertices.data());
    indexBuffer.initialize(context,
                           sizeof(vkt::Index) * indices.size(),
                           usage, properties, this->indices.data());
}
