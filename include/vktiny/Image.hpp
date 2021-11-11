#pragma once
#include "Context.hpp"
#include "CommandBuffer.hpp"

namespace vkt
{
    class Image
    {
    public:
        Image() = default;
        Image(const Image&) = delete;
        Image& operator = (const Image&) = delete;

        void initialize(const Context& context,
                        vk::Extent2D extent,
                        vk::ImageUsageFlags usage,
                        vk::Format format = vk::Format::eB8G8R8A8Unorm)
        {
            this->context = &context;

            vk::ImageCreateInfo imageInfo;
            imageInfo.setImageType(vk::ImageType::e2D);
            imageInfo.setExtent({ extent.width, extent.height, 1 });
            imageInfo.setMipLevels(1);
            imageInfo.setArrayLayers(1);
            imageInfo.setFormat(format);
            imageInfo.setUsage(usage);
            image = vk::raii::Image(context.getDevice(), imageInfo);

            using vkMP = vk::MemoryPropertyFlagBits;
            auto requirements = image.getMemoryRequirements();
            auto memoryType = context.findMemoryType(requirements, vkMP::eDeviceLocal);
            vk::MemoryAllocateInfo allocInfo{ requirements.size, memoryType };
            deviceMemory = vk::raii::DeviceMemory(context.getDevice(), allocInfo);
            image.bindMemory(*deviceMemory, 0);

            vk::ImageViewCreateInfo viewInfo;
            viewInfo.setImage(*image);
            viewInfo.setViewType(vk::ImageViewType::e2D);
            viewInfo.setFormat(format);
            viewInfo.setSubresourceRange({ vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 });
            view = vk::raii::ImageView(context.getDevice(), viewInfo);

            imageLayout = vk::ImageLayout::eUndefined;
        }

        static void transitionLayout(const vk::raii::CommandBuffer& cmdBuf,
                                     const vk::raii::Image& image,
                                     vk::ImageLayout oldLayout,
                                     vk::ImageLayout newLayout)
        {
            vk::PipelineStageFlags srcStageMask = vk::PipelineStageFlagBits::eAllCommands;
            vk::PipelineStageFlags dstStageMask = vk::PipelineStageFlagBits::eAllCommands;

            vk::ImageMemoryBarrier barrier{};
            barrier.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
            barrier.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
            barrier.setImage(*image);
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
            cmdBuf.pipelineBarrier(srcStageMask, dstStageMask, {}, {}, {}, barrier);
        }

        void transitionLayout(vk::ImageLayout newLayout)
        {
            context->OneTimeSubmitGraphics(
                [&](const vk::raii::CommandBuffer& commandBuffer) {
                    Image::transitionLayout(commandBuffer, image, imageLayout, newLayout);
                }
            );
        }

    protected:
        const Context* context;
        vk::raii::Image image = nullptr;
        vk::raii::DeviceMemory deviceMemory = nullptr;
        vk::raii::ImageView view = nullptr;
        vk::raii::Sampler sampler = nullptr;
        vk::ImageLayout imageLayout;
    };
}
