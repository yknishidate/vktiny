#pragma once

#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>

class Context;

namespace vkt
{
    class CommandBuffer
    {
    public:
        CommandBuffer(vk::Device device, vk::CommandPool commandPool, vk::Queue queue);

        CommandBuffer(const CommandBuffer&) = delete;
        CommandBuffer(CommandBuffer&&) = default;
        CommandBuffer& operator=(const CommandBuffer&) = delete;
        CommandBuffer& operator=(CommandBuffer&&) = default;

        void begin(vk::CommandBufferBeginInfo beginInfo = {});

        void end();

        void submit() const;

        vk::CommandBuffer get() const { return *commandBuffer; }

    protected:
        vk::UniqueCommandBuffer commandBuffer;
        vk::Queue queue;
    };
}
