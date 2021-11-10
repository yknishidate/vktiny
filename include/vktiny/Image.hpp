#pragma once
#include "Context.hpp"

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
        }

    protected:
        vk::raii::Image image = nullptr;
        vk::raii::DeviceMemory deviceMemory = nullptr;
        vk::raii::ImageView view = nullptr;
        vk::raii::Sampler sampler = nullptr;
    };
}
