#pragma once
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>
#include "vktiny/Vulkan/Instance.hpp"

namespace vkt
{
    class DebugMessenger
    {
    public:
        DebugMessenger() = default;
        DebugMessenger(const DebugMessenger&) = delete;
        DebugMessenger(DebugMessenger&&) = default;
        DebugMessenger& operator = (const DebugMessenger&) = delete;
        DebugMessenger& operator = (DebugMessenger&&) = default;

        void initialize(const Instance& instance);

    private:
        vk::UniqueDebugUtilsMessengerEXT messenger;
    };
}
