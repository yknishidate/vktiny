#pragma once
#include "vktiny/Math.hpp"

namespace vkt
{
    glm::mat4 flipY(const glm::mat4& transformMatrix)
    {
        glm::mat4 flipped = transformMatrix;
        flipped[1][1] *= -1.0; // flip y scale
        flipped[3][1] *= -1.0; // flip y translate
        return flipped;
    }
}
