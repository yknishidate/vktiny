#include <set>
#include <iostream>
#include "vktiny/Vulkan/Device.hpp"

namespace vkt
{
    void Device::initialize(const Instance& instance,
                            const PhysicalDevice& physicalDevice,
                            const Surface& surface,
                            std::vector<const char*> layers,
                            std::vector<const char*> extensions,
                            vk::PhysicalDeviceFeatures features,
                            void* pNext)
    {
        findQueueFamilies(physicalDevice.get(), surface.get());

        std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = { graphicsFamily, computeFamily, presentFamily };
        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies) {
            vk::DeviceQueueCreateInfo queueCreateInfo{ {}, queueFamily, 1, &queuePriority };
            queueCreateInfos.push_back(queueCreateInfo);
        }

        vk::DeviceCreateInfo createInfo{ {}, queueCreateInfos, layers, extensions, &features };
        createInfo.setPNext(pNext);
        device = physicalDevice.get().createDeviceUnique(createInfo);
        VULKAN_HPP_DEFAULT_DISPATCHER.init(*device);

        getQueues();
        createCommandPools();
    }

    void Device::findQueueFamilies(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface)
    {
        int i = 0;
        for (const auto& queueFamily : physicalDevice.getQueueFamilyProperties()) {
            if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
                graphicsFamily = i;
            }
            if (queueFamily.queueFlags & vk::QueueFlagBits::eCompute) {
                computeFamily = i;
            }
            vk::Bool32 presentSupport = physicalDevice.getSurfaceSupportKHR(i, surface);
            if (presentSupport) {
                presentFamily = i;
            }
            if (graphicsFamily != -1 && presentFamily != -1 && computeFamily != -1) {
                break;
            }
            i++;
        }
    }

    void Device::getQueues()
    {
        graphicsQueue = device->getQueue(graphicsFamily, 0);
        computeQueue = device->getQueue(computeFamily, 0);
        presentQueue = device->getQueue(presentFamily, 0);
    }

    void Device::createCommandPools()
    {
        vk::CommandPoolCreateInfo createInfo;
        createInfo.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);

        createInfo.setQueueFamilyIndex(graphicsFamily);
        graphicsCommandPool = device->createCommandPoolUnique(createInfo);

        createInfo.setQueueFamilyIndex(computeFamily);
        computeCommandPool = device->createCommandPoolUnique(createInfo);
    }

    vk::UniqueCommandBuffer Device::beginGraphicsCommand() const
    {
        vk::CommandBufferAllocateInfo allocInfo;
        allocInfo.setCommandPool(*graphicsCommandPool);
        allocInfo.setCommandBufferCount(1);
        auto commandBuffers = device->allocateCommandBuffersUnique(allocInfo);

        vk::UniqueCommandBuffer commandBuffer = std::move(commandBuffers.front());
        commandBuffer->begin({ vk::CommandBufferUsageFlagBits::eOneTimeSubmit });

        return commandBuffer;
    }

    vk::UniqueCommandBuffer Device::beginComputeCommand() const
    {
        vk::CommandBufferAllocateInfo allocInfo;
        allocInfo.setCommandPool(*computeCommandPool);
        allocInfo.setCommandBufferCount(1);
        auto commandBuffers = device->allocateCommandBuffersUnique(allocInfo);

        vk::UniqueCommandBuffer commandBuffer = std::move(commandBuffers.front());
        commandBuffer->begin({ vk::CommandBufferUsageFlagBits::eOneTimeSubmit });

        return commandBuffer;
    }

    void Device::endGraphicsCommand(vk::CommandBuffer commandBuffer) const
    {
        commandBuffer.end();

        vk::UniqueFence fence = device->createFenceUnique({});

        vk::SubmitInfo submitInfo;
        submitInfo.setCommandBuffers(commandBuffer);
        graphicsQueue.submit(submitInfo, *fence);

        if (device->waitForFences(*fence, true, UINT64_MAX) != vk::Result::eSuccess) {
            throw std::runtime_error("failed to wait for fences");
        }
    }

    void Device::endComputeCommand(vk::CommandBuffer commandBuffer) const
    {
        commandBuffer.end();

        vk::UniqueFence fence = device->createFenceUnique({});

        vk::SubmitInfo submitInfo;
        submitInfo.setCommandBuffers(commandBuffer);
        computeQueue.submit(submitInfo, *fence);

        if (device->waitForFences(*fence, true, UINT64_MAX) != vk::Result::eSuccess) {
            throw std::runtime_error("failed to wait for fences");
        }
    }
}
