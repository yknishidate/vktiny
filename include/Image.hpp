#pragma once

#include "Context.hpp"

class Image
{
public:
    void initialize(const Context& context,
                    vk::Extent2D extent, vk::Format format, vk::ImageUsageFlags usage);

    void createImageView();

    vk::WriteDescriptorSet createWrite();

    static void copyImage(vk::CommandBuffer cmdBuf, vk::Image srcImage, vk::Image dstImage,
                          vk::Extent2D extent);
    static void transitionImageLayout(vk::CommandBuffer cmdBuf, vk::Image image,
                                      vk::ImageLayout oldLayout, vk::ImageLayout newLayout);
    void transitionImageLayout(vk::ImageLayout newLayout);

    vk::Image get() const { return *image; }

private:
    void create(vk::Extent2D extent,
                vk::Format format, vk::ImageUsageFlags usage);
    void allocate();

    const Context* context;
    vk::UniqueImage image;
    vk::UniqueImageView view;
    vk::UniqueDeviceMemory memory;
    vk::Format format;
    vk::ImageLayout imageLayout;
    vk::DescriptorImageInfo imageInfo;
};
