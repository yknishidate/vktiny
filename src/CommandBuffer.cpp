#include "vktiny/Context.hpp"
#include "vktiny/CommandBuffer.hpp"

namespace vkt
{
    CommandBuffer::CommandBuffer(vk::Device device, vk::CommandPool commandPool, vk::Queue queue)
    {
        this->queue = queue;
        vk::CommandBufferAllocateInfo allocInfo;
        allocInfo.setCommandPool(commandPool);
        allocInfo.setCommandBufferCount(1);
        commandBuffer = std::move(device.allocateCommandBuffersUnique(allocInfo).front());
    }

    void CommandBuffer::begin(vk::CommandBufferBeginInfo beginInfo) const
    {
        commandBuffer->begin(beginInfo);
    }

    void CommandBuffer::end() const
    {
        commandBuffer->end();
    }

    void CommandBuffer::submit() const
    {
        vk::SubmitInfo submitInfo{ nullptr, nullptr, *commandBuffer };
        queue.submit(submitInfo, nullptr);
        queue.waitIdle();
    }
}
