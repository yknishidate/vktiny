#pragma once
#include "vktiny/Vulkan/DebugMessenger.hpp"
#include "vktiny/Vulkan/Device.hpp"
#include "vktiny/Vulkan/Swapchain.hpp"
#include "vktiny/Input.hpp"

namespace vkt
{
    class Context
    {
    public:
        Context() = default;
        Context(const Context&) = delete;
        Context(Context&&) = default;
        Context& operator = (const Context&) = delete;
        Context& operator = (Context&&) = default;

        ~Context();

        void initialize(uint32_t apiVersion,
                        bool enableValidationLayer,
                        int width, int height,
                        std::vector<const char*> deviceExtensions = {},
                        vk::PhysicalDeviceFeatures features = {},
                        void* deviceCreatePNext = nullptr);

        bool running() const;
        void pollEvents();

        auto& getSwapchain() { return swapchain; }
        auto getGLFWWindow() const { return window; }

        const auto& getDevice() const { return device; }
        const auto& getPhysicalDevice() const { return physicalDevice; }
        auto getVkDevice() const { return device.get(); }
        auto getVkPhysicalDevice() const { return physicalDevice.get(); }
        auto& getInput() { return input; }

        void setWindowIcon(const std::string& filepath);

    private:
        GLFWwindow* window = nullptr;

        Instance instance;
        DebugMessenger messenger;
        Surface surface;
        PhysicalDevice physicalDevice;
        Device device;
        Swapchain swapchain;
        Input input;
    };
}
