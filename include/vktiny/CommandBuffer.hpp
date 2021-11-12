#pragma once
#include "Context.hpp"

namespace vkt
{
    class CommandBuffer
    {
    public:
        CommandBuffer(const Context& context)
            : context(&context)
        {
        }

        CommandBuffer(const CommandBuffer&) = delete;
        CommandBuffer(CommandBuffer&&) = default;
        CommandBuffer& operator=(const CommandBuffer&) = delete;
        CommandBuffer& operator=(CommandBuffer&&) = default;

        void begin(vk::CommandBufferBeginInfo beginInfo = {})
        {
            commandBuffer->begin(beginInfo);
        }

        void end()
        {
            commandBuffer->end();
        }

        virtual void submit() const = 0;

    protected:
        const Context* context;
        vk::UniqueCommandBuffer commandBuffer;
    };

    class GraphicsCommandBuffer : public CommandBuffer
    {
        GraphicsCommandBuffer(const Context& context)
            : CommandBuffer(context)
        {
            vk::CommandBufferAllocateInfo allocInfo;
            allocInfo.setCommandPool(context.getGraphicsCommandPool());
            allocInfo.setCommandBufferCount(1);
            vk::Device device = context.getDevice();
            commandBuffer = std::move(device.allocateCommandBuffersUnique(allocInfo).front());
        }

        void submit() const override
        {
            vk::SubmitInfo submitInfo{ nullptr, nullptr, *commandBuffer };
            context->getGraphicsQueue().submit(submitInfo, nullptr);
            context->getGraphicsQueue().waitIdle();
        }
    };

    class ComputeCommandBuffer : public CommandBuffer
    {
        ComputeCommandBuffer(const Context& context)
            : CommandBuffer(context)
        {
            vk::CommandBufferAllocateInfo allocInfo;
            allocInfo.setCommandPool(context.getComputeCommandPool());
            allocInfo.setCommandBufferCount(1);
            vk::Device device = context.getDevice();
            commandBuffer = std::move(device.allocateCommandBuffersUnique(allocInfo).front());
        }

        void submit() const override
        {
            vk::SubmitInfo submitInfo{ nullptr, nullptr, *commandBuffer };
            context->getComputeQueue().submit(submitInfo, nullptr);
            context->getComputeQueue().waitIdle();
        }
    };
}
