#pragma once
#include "vktiny/Math.hpp"
#include <vulkan/vulkan.hpp>

namespace vkt
{
    glm::mat4 flipY(const glm::mat4& transformMatrix)
    {
        glm::mat4 flipped = transformMatrix;
        flipped[1][1] *= -1.0; // flip y scale
        flipped[3][1] *= -1.0; // flip y translate
        return flipped;
    }

    vk::TransformMatrixKHR toVkMatrix(const glm::mat4& transformMatrix)
    {
        const glm::mat4 transposedMatrix = glm::transpose(transformMatrix);
        std::array<std::array<float, 4>, 3> data;
        std::memcpy(&data, &transposedMatrix, sizeof(vk::TransformMatrixKHR));
        return vk::TransformMatrixKHR(data);
    }
}
