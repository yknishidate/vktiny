#include "vktiny/Image.hpp"
#include "vktiny/Buffer.hpp"
#include "vktiny/CommandBuffer.hpp"

namespace vkt
{
    Image::Image(const Context& context,
                 vk::Extent2D extent, vk::Format format, vk::ImageUsageFlags usage)
        : context(&context)
        , extent(extent)
        , format(format)
        , imageLayout(vk::ImageLayout::eUndefined)
    {
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
        image = context->getDevice().createImageUnique(createInfo);
    }

    void Image::allocate()
    {
        auto requirements = context->getDevice().getImageMemoryRequirements(*image);
        auto memoryType = context->findMemoryType(
            requirements.memoryTypeBits,
            vk::MemoryPropertyFlagBits::eDeviceLocal);
        memory = context->getDevice().allocateMemoryUnique({ requirements.size, memoryType });
        context->getDevice().bindImageMemory(*image, *memory, 0);
    }

    void Image::createImageView()
    {
        vk::ImageViewCreateInfo createInfo;
        createInfo.setImage(*image);
        createInfo.setViewType(vk::ImageViewType::e2D);
        createInfo.setFormat(format);
        createInfo.setSubresourceRange({ vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 });
        view = context->getDevice().createImageViewUnique(createInfo);
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
        sampler = context->getDevice().createSamplerUnique(samplerInfo);
    }

    //void Image::copyBuffer(const Buffer& buffer)
    //{
    //    context->OneTimeSubmitGraphics(
    //        [&](vk::CommandBuffer cmdBuf) {
    //            vk::BufferImageCopy region{};
    //            region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
    //            region.imageSubresource.mipLevel = 0;
    //            region.imageSubresource.baseArrayLayer = 0;
    //            region.imageSubresource.layerCount = 1;
    //            region.imageExtent = vk::Extent3D{ extent.width, extent.height, 1 };
    //            cmdBuf.copyBufferToImage(buffer.get(), *image, vk::ImageLayout::eTransferDstOptimal, region);
    //        });
    //}

    void Image::transitionLayout(vk::ImageLayout newLayout)
    {
        context->OneTimeSubmitGraphics(
            [&](const CommandBuffer& cmdBuf) {
                cmdBuf.transitionImageLayout(*image, imageLayout, newLayout);
            });
        imageLayout = newLayout;
    }
}
