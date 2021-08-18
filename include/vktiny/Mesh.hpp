#pragma once
#include "Buffer.hpp"
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

    private:

    };
}
