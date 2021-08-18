#pragma once
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>
#include "vktiny/Vulkan/Device.hpp"
#include <utility>

namespace vkt
{
    struct FrameInfo
    {
        uint32_t imageIndex;
        vk::Semaphore imageAvailableSemaphore;
        vk::Semaphore renderFinishedSemaphore;
        vk::Fence inFlightFence;
    };

    class Swapchain
    {
    public:
        ~Swapchain()
        {
            for (size_t i = 0; i < maxFramesInFlight; i++) {
                device->get().destroyFence(inFlightFences[i]);
            }
        }
        void initialize(const Device& device,
                        const PhysicalDevice& physicalDevice,
                        const Surface& surface,
                        int width, int height);

        vk::SwapchainKHR get() const { return swapchain.get(); }

        auto getExtent() const { return imageExtent; }
        auto getFormat() const { return imageFormat; }
        auto getChainSize() const { return images.size(); }
        const auto& getImages() const { return images; }

        auto allocateDrawComamndBuffers() const
        {
            vk::CommandBufferAllocateInfo allocInfo;
            allocInfo.setCommandPool(device->getGraphicsCommandPool());
            allocInfo.setLevel(vk::CommandBufferLevel::ePrimary);
            allocInfo.setCommandBufferCount(images.size());
            return device->get().allocateCommandBuffersUnique(allocInfo);
        }

        uint32_t acquireNextImageIndex() const
        {
            auto res = device->get().acquireNextImageKHR(*swapchain, UINT64_MAX,
                                                         *imageAvailableSemaphores[currentFrame]);
            if (res.result == vk::Result::eSuccess) {
                return res.value;
            }
            throw std::runtime_error("failed to acquire next image!");
        }

        FrameInfo beginFrame()
        {
            device->get().waitForFences(inFlightFences[currentFrame], true, UINT64_MAX);

            uint32_t imageIndex = acquireNextImageIndex();

            if (imagesInFlight[imageIndex]) {
                device->get().waitForFences(imagesInFlight[imageIndex], true, UINT64_MAX);
            }
            imagesInFlight[imageIndex] = inFlightFences[currentFrame];
            device->get().resetFences(inFlightFences[currentFrame]);

            FrameInfo frameInfo;
            frameInfo.imageIndex = imageIndex;
            frameInfo.imageAvailableSemaphore = *imageAvailableSemaphores[currentFrame];
            frameInfo.renderFinishedSemaphore = *renderFinishedSemaphores[currentFrame];
            frameInfo.inFlightFence = inFlightFences[currentFrame];
            return frameInfo;
        }

        void endFrame(uint32_t imageIndex, vk::CommandBuffer cmdBuf)
        {
            device->getPresentQueue().presentKHR(
                vk::PresentInfoKHR{}
                .setWaitSemaphores(*renderFinishedSemaphores[currentFrame])
                .setSwapchains(*swapchain)
                .setImageIndices(imageIndex));

            currentFrame = (currentFrame + 1) % maxFramesInFlight;
        }

    private:
        void createViews();
        void createSyncObjects();

        const Device* device;

        vk::UniqueSwapchainKHR swapchain;
        std::vector<vk::Image> images;
        vk::Format imageFormat;
        vk::Extent2D imageExtent;
        std::vector<vk::UniqueImageView> imageViews;
        std::vector<vk::UniqueFramebuffer> framebuffers;

        size_t currentFrame = 0;
        const int maxFramesInFlight = 2;
        std::vector<vk::UniqueSemaphore> imageAvailableSemaphores;
        std::vector<vk::UniqueSemaphore> renderFinishedSemaphores;
        std::vector<vk::Fence> inFlightFences;
        std::vector<vk::Fence> imagesInFlight;
    };
}
