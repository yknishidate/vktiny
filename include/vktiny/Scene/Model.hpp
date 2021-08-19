#pragma once
#include "vktiny/Math.hpp"

namespace vkt
{
    class Model
    {
        int meshIndex{ -1 };

        glm::mat4 worldMatrix{ 1.0f };
        glm::vec3 translation{ 1.0f };
        glm::quat rotation{ 1.0f, 0.0f, 0.0f, 0.0f };
        glm::vec3 scale{ 1.0f, 1.0f, 1.0f };
    };
}
