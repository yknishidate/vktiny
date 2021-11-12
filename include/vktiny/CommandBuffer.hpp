#pragma once

#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>
#include "Pipeline.hpp"
#include "DescriptorSet.hpp"

namespace vkt
{
    class Context;

    class CommandBuffer
    {
    public:
        CommandBuffer(vk::UniqueCommandBuffer commandBuffer)
            : commandBuffer(std::move(commandBuffer))
        {
        }

        CommandBuffer(vk::Device device, vk::CommandPool commandPool, vk::Queue queue);

        CommandBuffer(const CommandBuffer&) = delete;
        CommandBuffer(CommandBuffer&&) = default;
        CommandBuffer& operator=(const CommandBuffer&) = delete;
        CommandBuffer& operator=(CommandBuffer&&) = default;

        void begin(vk::CommandBufferBeginInfo beginInfo = {}) const;
        void end() const;
        void submit() const;

        void dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
        {
            commandBuffer->dispatch(groupCountX, groupCountY, groupCountZ);
        }

        void bindPipeline(const Pipeline& pipeline)
        {
            commandBuffer->bindPipeline(pipeline.getBindPoint(), pipeline.get());
        }

        void bindDescriptorSets(const DescriptorSet& descSet, const Pipeline& pipeline)
        {
            vk::PipelineBindPoint bindPoint = pipeline.getBindPoint();
            vk::PipelineLayout layout = pipeline.getLayout();
            commandBuffer->bindDescriptorSets(bindPoint, layout, 0, descSet.get(), nullptr);
        }

        vk::CommandBuffer get() const { return *commandBuffer; }

    protected:
        vk::UniqueCommandBuffer commandBuffer;
        vk::Queue queue;
    };
}
