#pragma once
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>
#include <utility>
#include "Context.hpp"

namespace vkt
{
    struct FrameInfo
    {
        uint32_t imageIndex;
        uint32_t currentFrame;
        vk::Semaphore imageAvailableSemaphore;
        vk::Semaphore renderFinishedSemaphore;
        vk::Fence inFlightFence;
    };

    class Swapchain
    {
    public:
        Swapchain(const Context& context, int width, int height);

        Swapchain(const Swapchain&) = delete;
        Swapchain(Swapchain&&) = default;
        Swapchain& operator=(const Swapchain&) = delete;
        Swapchain& operator=(Swapchain&&) = default;

        ~Swapchain();

        std::vector<vk::UniqueCommandBuffer> allocateDrawComamndBuffers() const;

        uint32_t acquireNextImageIndex() const;

        FrameInfo beginFrame();

        void endFrame(uint32_t imageIndex);

        vk::SwapchainKHR get() const { return swapchain.get(); }

        auto getExtent() const { return imageExtent; }
        auto getFormat() const { return imageFormat; }
        auto getImagesSize() const { return images.size(); }
        const auto& getImages() const { return images; }

    private:
        void createViews();
        void createSyncObjects();

        const Context* context;

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
