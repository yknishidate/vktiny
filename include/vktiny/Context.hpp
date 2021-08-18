#pragma once
#include "DebugMessenger.hpp"
#include "Device.hpp"
#include "Swapchain.hpp"

namespace vkt
{
    class Context
    {
    public:
        ~Context();

        void initialize(uint32_t apiVersion,
                        bool enableValidationLayer,
                        int width, int height,
                        std::vector<const char*> deviceExtensions = {},
                        void* deviceCreatePNext = nullptr);

        bool running() const;
        void pollEvents() const;

        auto& getSwapchain() { return swapchain; }

        const auto& getDevice() const { return device; }
        const auto& getPhysicalDevice() const { return physicalDevice; }
        auto getVkDevice() const { return device.get(); }
        auto getVkPhysicalDevice() const { return physicalDevice.get(); }

    private:
        GLFWwindow* window;

        Instance instance;
        DebugMessenger messenger;
        Surface surface;
        PhysicalDevice physicalDevice;
        Device device;
        Swapchain swapchain;
    };
}
