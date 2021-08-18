#include "vktiny/Image.hpp"

namespace vkt
{
    void Image::initialize(const Context& context,
                           vk::Extent2D extent, vk::Format format, vk::ImageUsageFlags usage)
    {
        this->context = &context;
        this->format = format;
        this->imageLayout = vk::ImageLayout::eUndefined;
        create(extent, format, usage);
        allocate();
    }

    void Image::create(vk::Extent2D extent, vk::Format format, vk::ImageUsageFlags usage)
    {
        vk::ImageCreateInfo createInfo;
        createInfo.setImageType(vk::ImageType::e2D);
        createInfo.setExtent({ extent.width, extent.height, 1 });
        createInfo.setMipLevels(1);
        createInfo.setArrayLayers(1);
        createInfo.setFormat(format);
        createInfo.setTiling(vk::ImageTiling::eOptimal);
        createInfo.setUsage(usage);
        image = context->getVkDevice().createImageUnique(createInfo);
    }

    void Image::allocate()
    {
        auto requirements = context->getVkDevice().getImageMemoryRequirements(*image);
        auto memoryType = context->getPhysicalDevice().findMemoryType(
            requirements.memoryTypeBits,
            vk::MemoryPropertyFlagBits::eDeviceLocal);
        memory = context->getVkDevice().allocateMemoryUnique({ requirements.size, memoryType });
        context->getVkDevice().bindImageMemory(*image, *memory, 0);
    }

    void Image::createImageView()
    {
        vk::ImageViewCreateInfo createInfo;
        createInfo.setImage(*image);
        createInfo.setViewType(vk::ImageViewType::e2D);
        createInfo.setFormat(format);
        createInfo.setSubresourceRange({ vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 });
        view = context->getVkDevice().createImageViewUnique(createInfo);
    }

    vk::WriteDescriptorSet Image::createWrite()
    {
        imageInfo = vk::DescriptorImageInfo{ {}, *view, imageLayout };

        vk::WriteDescriptorSet imageWrite;
        imageWrite.setDescriptorCount(1);
        imageWrite.setImageInfo(imageInfo);
        return imageWrite;
    }

    void Image::copyImage(vk::CommandBuffer cmdBuf, vk::Image srcImage, vk::Image dstImage,
                          vk::Extent2D extent)
    {
        using vkIL = vk::ImageLayout;
        vk::ImageCopy copyRegion{};
        copyRegion.setSrcSubresource({ vk::ImageAspectFlagBits::eColor, 0, 0, 1 });
        copyRegion.setDstSubresource({ vk::ImageAspectFlagBits::eColor, 0, 0, 1 });
        copyRegion.setExtent({ extent.width, extent.height, 1 });
        cmdBuf.copyImage(srcImage, vkIL::eTransferSrcOptimal,
                         dstImage, vkIL::eTransferDstOptimal, copyRegion);
    }

    void Image::transitionLayout(vk::CommandBuffer cmdBuf, vk::Image image,
                                 vk::ImageLayout oldLayout, vk::ImageLayout newLayout)
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
        cmdBuf.pipelineBarrier(srcStageMask, dstStageMask, {}, {}, {}, barrier);
    }

    void Image::transitionLayout(vk::ImageLayout newLayout)
    {
        vk::UniqueCommandBuffer cmdBuf = context->getDevice().beginGraphicsCommand();
        Image::transitionLayout(*cmdBuf, *image, imageLayout, newLayout);
        context->getDevice().endGraphicsCommand(*cmdBuf);
        imageLayout = newLayout;
    }
}
