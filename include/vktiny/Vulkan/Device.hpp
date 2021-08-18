#pragma once
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>
#include "vktiny/Vulkan/PhysicalDevice.hpp"
#include "vktiny/Vulkan/Surface.hpp"

namespace vkt
{
    class Device
    {
    public:
        void initialize(const Instance& instance,
                        const PhysicalDevice& physicalDevice,
                        const Surface& surface,
                        std::vector<const char*> layers = {},
                        std::vector<const char*> extensions = {},
                        vk::PhysicalDeviceFeatures features = {},
                        void* pNext = nullptr);

        vk::Device get() const { return *device; }

        vk::CommandPool getGraphicsCommandPool() const { return *graphicsCommandPool; }
        vk::CommandPool getComputeCommandPool() const { return *computeCommandPool; }

        uint32_t getGraphicsFamily() const { return graphicsFamily; }
        uint32_t getPresentFamily() const { return presentFamily; }
        uint32_t getComputeFamily() const { return computeFamily; }

        vk::Queue getGraphicsQueue() const { return graphicsQueue; }
        vk::Queue getPresentQueue() const { return presentQueue; }
        vk::Queue getComputeQueue() const { return computeQueue; }

        vk::UniqueCommandBuffer beginGraphicsCommand() const;
        vk::UniqueCommandBuffer beginComputeCommand() const;
        void endGraphicsCommand(vk::CommandBuffer commandBuffer) const;
        void endComputeCommand(vk::CommandBuffer commandBuffer) const;

        void waitIdle() const { device->waitIdle(); }

    private:
        void findQueueFamilies(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface);
        void getQueues();
        void createCommandPools();

        vk::UniqueDevice device;

        vk::UniqueCommandPool graphicsCommandPool;
        vk::UniqueCommandPool computeCommandPool;

        uint32_t graphicsFamily;
        uint32_t presentFamily;
        uint32_t computeFamily;

        vk::Queue graphicsQueue;
        vk::Queue presentQueue;
        vk::Queue computeQueue;
    };
}
