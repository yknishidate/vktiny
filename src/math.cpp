#pragma once
#include "vktiny/math.hpp"

namespace vkt
{
    glm::mat4 flipY(const glm::mat4& transformMatrix)
    {
        glm::mat4 flipped = transformMatrix;
        flipped[1][1] *= -1.0;
        return flipped;
    }
}
