#pragma once
#include "Context.hpp"

namespace vkt
{
    class Buffer;

    class Image
    {
    public:
        Image(const Context& context,
              vk::Extent2D extent,
              vk::Format format,
              vk::ImageUsageFlags usage);
        Image(const Image&) = delete;
        Image(Image&&) = default;
        Image& operator=(const Image&) = delete;
        Image& operator=(Image&&) = default;

        void createImageView();
        void createSampler();

        //void copyBuffer(const Buffer& buffer);

        void transitionLayout(vk::ImageLayout newLayout);

        vk::Image get() const { return *image; }
        vk::ImageView getView() const { return *view; }
        vk::ImageLayout getLayout() const { return imageLayout; }

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
