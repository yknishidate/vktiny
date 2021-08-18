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

    struct Material
    {
        int baseColorTextureIndex{ -1 };
        int metallicRoughnessTextureIndex{ -1 };
        int normalTextureIndex{ -1 };
        int occlusionTextureIndex{ -1 };
        int emissiveTextureIndex{ -1 };

        glm::vec4 baseColorFactor{ 1.0f };
        float metallicFactor{ 1.0f };
        float roughnessFactor{ 1.0f };
        glm::vec3 emissiveFactor{ 0.0f };
    };

    class Mesh
    {
    public:

    private:

    };
}
