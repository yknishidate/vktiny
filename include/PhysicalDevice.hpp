#pragma once
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>
#include "Instance.hpp"

class PhysicalDevice
{
public:
    void initialize(const Instance& instance);

    vk::PhysicalDevice get() const { return physicalDevice; }

    uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) const;

private:
    vk::PhysicalDevice physicalDevice;
};
