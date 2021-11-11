#pragma once
#include "Context.hpp"

namespace vkt
{
    class CommandBuffer
    {
    public:
        CommandBuffer() = default;
        CommandBuffer(const CommandBuffer&) = delete;
        CommandBuffer& operator=(const CommandBuffer&) = delete;

        void initialize(const Context& context)
        {
            vk::CommandBufferAllocateInfo allocInfo;
            allocInfo.setCommandPool(*context.getGraphicsCommandPool());
            allocInfo.setCommandBufferCount(1);
            auto commandBuffers = vk::raii::CommandBuffers(context.getDevice(), allocInfo);
            commandBuffer = std::move(commandBuffers.front());
        }

        void begin()
        {
            commandBuffer.begin({ vk::CommandBufferUsageFlagBits::eOneTimeSubmit });
        }

        const vk::raii::CommandBuffer& get() const { return commandBuffer; }

    protected:
        vk::raii::CommandBuffer commandBuffer = nullptr;
    };
}
