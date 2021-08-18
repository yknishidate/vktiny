#pragma once
#include "vktiny/Vulkan/Context.hpp"

namespace vkt
{
    class Buffer;

    class Image
    {
    public:
        void initialize(const Context& context,
                        vk::Extent2D extent,
                        vk::Format format,
                        vk::ImageUsageFlags usage);

        void createImageView();
        void createSampler();

        vk::DescriptorImageInfo getDescInfo();
        vk::WriteDescriptorSet createWrite();

        static void copyImage(vk::CommandBuffer cmdBuf,
                              vk::Image srcImage,
                              vk::Image dstImage,
                              vk::Extent2D extent);

        void copyBuffer(const Buffer& buffer);

        static void transitionLayout(vk::CommandBuffer cmdBuf,
                                     vk::Image image,
                                     vk::ImageLayout oldLayout,
                                     vk::ImageLayout newLayout);

        void transitionLayout(vk::ImageLayout newLayout);

        vk::Image get() const { return *image; }

    private:
        void create(vk::ImageUsageFlags usage);
        void allocate();

        const Context* context;
        vk::UniqueImage image;
        vk::UniqueImageView view;
        vk::UniqueSampler sampler;

        vk::UniqueDeviceMemory memory;
        vk::Extent2D extent;
        vk::Format format;
        vk::ImageLayout imageLayout;
        vk::DescriptorImageInfo imageInfo;
    };
}
