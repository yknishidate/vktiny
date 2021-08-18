#pragma once
#include <glm/glm.hpp>

namespace vkt
{
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
}
