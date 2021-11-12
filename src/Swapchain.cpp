#include <iostream>
#include "vktiny/Swapchain.hpp"

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
            vk::Extent2D extent{ static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
            vk::Extent2D minExtent = capabilities.minImageExtent;
            vk::Extent2D maxExtent = capabilities.maxImageExtent;
            extent.width = std::clamp(extent.width, minExtent.width, maxExtent.width);
            extent.height = std::clamp(extent.height, minExtent.height, maxExtent.height);
            return extent;
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

    Swapchain::~Swapchain()
    {
        for (size_t i = 0; i < maxFramesInFlight; i++) {
            context->getDevice().destroyFence(inFlightFences[i]);
        }
    }

    Swapchain::Swapchain(const Context& context, int width, int height)
        : context(&context)
    {
        vk::Device device = context.getDevice();
        vk::PhysicalDevice physicalDevice = context.getPhysicalDevice();
        vk::SurfaceKHR surface = context.getSurface();
        auto swapChainSupport = querySwapChainSupport(physicalDevice, surface);

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
        createInfo.setSurface(surface);
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

        if (context.getGraphicsFamily() != context.getPresentFamily()) {
            std::array familyIndices{ context.getGraphicsFamily(), context.getPresentFamily() };
            createInfo.setImageSharingMode(vk::SharingMode::eConcurrent);
            createInfo.setQueueFamilyIndices(familyIndices);
        }

        swapchain = device.createSwapchainKHRUnique(createInfo);
        images = device.getSwapchainImagesKHR(*swapchain);
        imageFormat = surfaceFormat.format;
        imageExtent = extent;
        createViews();
        createSyncObjects();
    }

    std::vector<vk::UniqueCommandBuffer> Swapchain::allocateDrawComamndBuffers() const
    {
        vk::CommandBufferAllocateInfo allocInfo;
        allocInfo.setCommandPool(context->getGraphicsCommandPool());
        allocInfo.setLevel(vk::CommandBufferLevel::ePrimary);
        allocInfo.setCommandBufferCount(images.size());
        return context->getDevice().allocateCommandBuffersUnique(allocInfo);
    }

    uint32_t Swapchain::acquireNextImageIndex() const
    {
        vk::Semaphore semaphore = *imageAvailableSemaphores[currentFrame];
        auto res = context->getDevice().acquireNextImageKHR(*swapchain, UINT64_MAX, semaphore);
        if (res.result != vk::Result::eSuccess) {
            throw std::runtime_error("failed to acquire next image!");
        }
        return res.value;
    }

    FrameInfo Swapchain::beginFrame()
    {
        context->getDevice().waitForFences(inFlightFences[currentFrame], true, UINT64_MAX);

        uint32_t imageIndex = acquireNextImageIndex();

        if (imagesInFlight[imageIndex]) {
            context->getDevice().waitForFences(imagesInFlight[imageIndex], true, UINT64_MAX);
        }
        imagesInFlight[imageIndex] = inFlightFences[currentFrame];
        context->getDevice().resetFences(inFlightFences[currentFrame]);

        FrameInfo frameInfo;
        frameInfo.imageIndex = imageIndex;
        frameInfo.currentFrame = currentFrame;
        frameInfo.imageAvailableSemaphore = *imageAvailableSemaphores[currentFrame];
        frameInfo.renderFinishedSemaphore = *renderFinishedSemaphores[currentFrame];
        frameInfo.inFlightFence = inFlightFences[currentFrame];
        return frameInfo;
    }

    void Swapchain::endFrame(uint32_t imageIndex)
    {
        context->getPresentQueue().presentKHR(
            vk::PresentInfoKHR{}
            .setWaitSemaphores(*renderFinishedSemaphores[currentFrame])
            .setSwapchains(*swapchain)
            .setImageIndices(imageIndex));

        currentFrame = (currentFrame + 1) % maxFramesInFlight;
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
            imageViews[i] = context->getDevice().createImageViewUnique(createInfo);
        }
    }

    void Swapchain::createSyncObjects()
    {
        imageAvailableSemaphores.resize(maxFramesInFlight);
        renderFinishedSemaphores.resize(maxFramesInFlight);
        inFlightFences.resize(maxFramesInFlight);
        imagesInFlight.resize(images.size());

        vk::Device device = context->getDevice();
        for (size_t i = 0; i < maxFramesInFlight; i++) {
            imageAvailableSemaphores[i] = device.createSemaphoreUnique({});
            renderFinishedSemaphores[i] = device.createSemaphoreUnique({});
            inFlightFences[i] = device.createFence({ vk::FenceCreateFlagBits::eSignaled });
        }
    }
}
