#pragma once
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

namespace vk
{
    struct TransformMatrixKHR;
}

namespace vkt
{
    glm::mat4 flipY(const glm::mat4& transformMatrix);
    vk::TransformMatrixKHR toVkMatrix(const glm::mat4& transformMatrix);
}
