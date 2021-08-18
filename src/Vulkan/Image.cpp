#include "vktiny/Vulkan/Image.hpp"
#include "vktiny/Vulkan/Buffer.hpp"

namespace vkt
{
    void Image::initialize(const Context& context,
                           vk::Extent2D extent, vk::Format format, vk::ImageUsageFlags usage)
    {
        this->context = &context;
        this->extent = extent;
        this->format = format;
        this->imageLayout = vk::ImageLayout::eUndefined;
        create(usage);
        allocate();
    }

    void Image::create(vk::ImageUsageFlags usage)
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

    void Image::createSampler()
    {
        vk::SamplerCreateInfo samplerInfo{};
        samplerInfo.magFilter = vk::Filter::eLinear;
        samplerInfo.minFilter = vk::Filter::eLinear;
        samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
        samplerInfo.addressModeU = vk::SamplerAddressMode::eMirroredRepeat;
        samplerInfo.addressModeV = vk::SamplerAddressMode::eMirroredRepeat;
        samplerInfo.addressModeW = vk::SamplerAddressMode::eMirroredRepeat;
        samplerInfo.compareOp = vk::CompareOp::eNever;
        samplerInfo.borderColor = vk::BorderColor::eFloatOpaqueWhite;
        samplerInfo.maxAnisotropy = 1.0;
        samplerInfo.anisotropyEnable = false;
        samplerInfo.maxLod = 1.0f;
        sampler = context->getVkDevice().createSamplerUnique(samplerInfo);
    }

    vk::DescriptorImageInfo Image::getDescInfo()
    {
        imageInfo = vk::DescriptorImageInfo{ *sampler, *view, imageLayout };
        return imageInfo;
    }

    vk::WriteDescriptorSet Image::createWrite()
    {
        imageInfo = vk::DescriptorImageInfo{ *sampler, *view, imageLayout };

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

    void Image::copyBuffer(const Buffer& buffer)
    {
        vk::UniqueCommandBuffer cmdBuf = context->getDevice().beginGraphicsCommand();

        vk::BufferImageCopy region{};
        region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageExtent = vk::Extent3D{ extent.width, extent.height, 1 };
        cmdBuf->copyBufferToImage(buffer.get(), *image, vk::ImageLayout::eTransferDstOptimal, region);

        context->getDevice().endGraphicsCommand(*cmdBuf);
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
