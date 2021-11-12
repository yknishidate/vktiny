#pragma once

#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>
#include "Pipeline.hpp"
#include "DescriptorSet.hpp"
#include "Image.hpp"

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

        // TODO: add vk::CommandBuffer's functions

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

        void copyImage(vk::Image srcImage, vk::Image dstImage, vk::Extent2D extent)
        {
            vk::ImageCopy copyRegion{};
            copyRegion.setSrcSubresource({ vk::ImageAspectFlagBits::eColor, 0, 0, 1 });
            copyRegion.setDstSubresource({ vk::ImageAspectFlagBits::eColor, 0, 0, 1 });
            copyRegion.setExtent({ extent.width, extent.height, 1 });

            auto srcLayout = vk::ImageLayout::eTransferSrcOptimal;
            auto dstLayout = vk::ImageLayout::eTransferDstOptimal;
            commandBuffer->copyImage(srcImage, srcLayout, dstImage, dstLayout, copyRegion);
        }

        void transitionImageLayout(vk::Image image,
                                   vk::ImageLayout oldLayout,
                                   vk::ImageLayout newLayout) const
        {
            vk::PipelineStageFlags srcStageMask = vk::PipelineStageFlagBits::eAllCommands;
            vk::PipelineStageFlags dstStageMask = vk::PipelineStageFlagBits::eAllCommands;

            vk::ImageMemoryBarrier barrier{};
            barrier.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
            barrier.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
            barrier.setImage(image);
            barrier.setOldLayout(oldLayout);
            barrier.setNewLayout(newLayout);
            barrier.setSubresourceRange({ vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 });

            using vkAF = vk::AccessFlagBits;
            switch (oldLayout) {
                case vk::ImageLayout::eTransferSrcOptimal:
                    barrier.srcAccessMask = vkAF::eTransferRead;
                    break;
                case vk::ImageLayout::eTransferDstOptimal:
                    barrier.srcAccessMask = vkAF::eTransferWrite;
                    break;
                case vk::ImageLayout::eShaderReadOnlyOptimal:
                    barrier.srcAccessMask = vkAF::eShaderRead;
                    break;
                default:
                    break;
            }

            switch (newLayout) {
                case vk::ImageLayout::eTransferDstOptimal:
                    barrier.dstAccessMask = vkAF::eTransferWrite;
                    break;
                case vk::ImageLayout::eTransferSrcOptimal:
                    barrier.dstAccessMask = vkAF::eTransferRead;
                    break;
                case vk::ImageLayout::eShaderReadOnlyOptimal:
                    if (barrier.srcAccessMask == vk::AccessFlags{}) {
                        barrier.srcAccessMask = vkAF::eHostWrite | vkAF::eTransferWrite;
                    }
                    barrier.dstAccessMask = vkAF::eShaderRead;
                    break;
                default:
                    break;
            }
            commandBuffer->pipelineBarrier(srcStageMask, dstStageMask, {}, {}, {}, barrier);
        }

        vk::CommandBuffer get() const { return *commandBuffer; }

    protected:
        vk::UniqueCommandBuffer commandBuffer;
        vk::Queue queue;
    };
}
