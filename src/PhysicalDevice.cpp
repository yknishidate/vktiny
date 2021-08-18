#include <iostream>
#include "PhysicalDevice.hpp"

namespace vkt
{
    void PhysicalDevice::initialize(const Instance& instance)
    {
        physicalDevice = instance.get().enumeratePhysicalDevices().front();
    }

    uint32_t PhysicalDevice::findMemoryType(uint32_t typeFilter,
                                            vk::MemoryPropertyFlags properties) const
    {
        vk::PhysicalDeviceMemoryProperties memProperties = physicalDevice.getMemoryProperties();
        for (uint32_t i = 0; i != memProperties.memoryTypeCount; ++i) {
            if ((typeFilter & (1 << i)) &&
                (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }
        throw std::runtime_error("failed to find suitable memory type");
    }
}
