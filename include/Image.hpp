#pragma once
#include "Device.hpp"

class Image
{
public:
    void initialize(const Device& device, const PhysicalDevice& physicalDevice,
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
    void create(vk::Device device, vk::Extent2D extent,
                vk::Format format, vk::ImageUsageFlags usage);
    void allocate(vk::Device device, const PhysicalDevice& physicalDevice);

    const Device* device;
    vk::UniqueImage image;
    vk::UniqueImageView view;
    vk::UniqueDeviceMemory memory;
    vk::Format format;
    vk::ImageLayout imageLayout;
    vk::DescriptorImageInfo imageInfo;
};
