#pragma once
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>
#include <spdlog/spdlog.h>

#include "vktiny/Vulkan/Context.hpp"
#include "vktiny/Vulkan/Pipeline.hpp"
#include "vktiny/Scene/Scene.hpp"

namespace vkt
{
    namespace log
    {
        using namespace spdlog;
    }
}
