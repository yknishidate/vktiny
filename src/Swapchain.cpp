#include <iostream>
#include "Swapchain.hpp"

namespace vkt
{
    struct SupportDetails
    {
        vk::SurfaceCapabilitiesKHR capabilities;
        std::vector<vk::SurfaceFormatKHR> formats;
        std::vector<vk::PresentModeKHR> presentModes;
    };

    vk::SurfaceFormatKHR chooseSwapSurfaceFormat(
        const std::vector<vk::SurfaceFormatKHR>& availableFormats)
    {
        if (availableFormats.size() == 1 &&
            availableFormats[0].format == vk::Format::eUndefined) {
            return { vk::Format::eB8G8R8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear };
        }
        for (const auto& format : availableFormats) {
            if (format.format == vk::Format::eB8G8R8A8Unorm &&
                format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
                return format;
            }
        }
        throw std::runtime_error("found no suitable surface format");
    }

    vk::PresentModeKHR chooseSwapPresentMode(
        const std::vector<vk::PresentModeKHR>& availablePresentModes)
    {
        for (const auto& availablePresentMode : availablePresentModes) {
            if (availablePresentMode == vk::PresentModeKHR::eFifoRelaxed) {
                return availablePresentMode;
            }
        }
        return vk::PresentModeKHR::eFifo;
    }

    vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities,
                                  int width, int height)
    {
        if (capabilities.currentExtent.width != UINT32_MAX) {
            return capabilities.currentExtent;
        } else {
            vk::Extent2D actualExtent{
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height) };
            vk::Extent2D minExtent = capabilities.minImageExtent;
            vk::Extent2D maxExtent = capabilities.maxImageExtent;
            actualExtent.width = std::clamp(actualExtent.width,
                                            minExtent.width, maxExtent.width);
            actualExtent.height = std::clamp(actualExtent.height,
                                             minExtent.height, maxExtent.height);
            return actualExtent;
        }
    }

    SupportDetails querySwapChainSupport(vk::PhysicalDevice device, vk::SurfaceKHR surface)
    {
        SupportDetails details;
        details.capabilities = device.getSurfaceCapabilitiesKHR(surface);
        details.formats = device.getSurfaceFormatsKHR(surface);
        details.presentModes = device.getSurfacePresentModesKHR(surface);
        return details;
    }

    void Swapchain::initialize(const Device& device,
                               const PhysicalDevice& physicalDevice,
                               const Surface& surface,
                               int width, int height)
    {
        this->device = &device;

        auto swapChainSupport = querySwapChainSupport(physicalDevice.get(), surface.get());

        vk::SurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
        vk::PresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
        vk::Extent2D extent = chooseSwapExtent(swapChainSupport.capabilities, width, height);

        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
        if (swapChainSupport.capabilities.maxImageCount > 0 &&
            imageCount > swapChainSupport.capabilities.maxImageCount) {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }

        using vkIU = vk::ImageUsageFlagBits;
        vk::SwapchainCreateInfoKHR createInfo;
        createInfo.setSurface(surface.get());
        createInfo.setMinImageCount(imageCount);
        createInfo.setImageFormat(surfaceFormat.format);
        createInfo.setImageColorSpace(surfaceFormat.colorSpace);
        createInfo.setImageExtent(extent);
        createInfo.setImageArrayLayers(1);
        createInfo.setImageUsage(vkIU::eColorAttachment | vkIU::eTransferDst);
        createInfo.setImageSharingMode(vk::SharingMode::eExclusive);
        createInfo.setPreTransform(swapChainSupport.capabilities.currentTransform);
        createInfo.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque);
        createInfo.setPresentMode(presentMode);
        createInfo.setClipped(VK_TRUE);

        if (device.getGraphicsFamily() != device.getPresentFamily()) {
            std::array familyIndices{ device.getGraphicsFamily(), device.getPresentFamily() };
            createInfo.setImageSharingMode(vk::SharingMode::eConcurrent);
            createInfo.setQueueFamilyIndices(familyIndices);
        }

        swapchain = device.get().createSwapchainKHRUnique(createInfo);
        images = device.get().getSwapchainImagesKHR(*swapchain);
        imageFormat = surfaceFormat.format;
        imageExtent = extent;
        createViews();
        createSyncObjects();
    }

    void Swapchain::createViews()
    {
        imageViews.resize(images.size());

        vk::ImageSubresourceRange subresourceRange{ vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 };
        for (size_t i = 0; i < images.size(); i++) {
            vk::ImageViewCreateInfo createInfo;
            createInfo.setImage(images[i]);
            createInfo.setViewType(vk::ImageViewType::e2D);
            createInfo.setFormat(imageFormat);
            createInfo.setSubresourceRange(subresourceRange);
            imageViews[i] = device->get().createImageViewUnique(createInfo);
        }
    }

    void Swapchain::createSyncObjects()
    {
        imageAvailableSemaphores.resize(maxFramesInFlight);
        renderFinishedSemaphores.resize(maxFramesInFlight);
        inFlightFences.resize(maxFramesInFlight);
        imagesInFlight.resize(images.size());

        for (size_t i = 0; i < maxFramesInFlight; i++) {
            imageAvailableSemaphores[i] = device->get().createSemaphoreUnique({});
            renderFinishedSemaphores[i] = device->get().createSemaphoreUnique({});
            inFlightFences[i] = device->get().createFence({ vk::FenceCreateFlagBits::eSignaled });
        }
    }
}