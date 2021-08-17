#pragma once
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>
#include "Instance.hpp"

class DebugMessenger
{
public:
    void initialize(const Instance& instance);

private:
    vk::UniqueDebugUtilsMessengerEXT messenger;
};
