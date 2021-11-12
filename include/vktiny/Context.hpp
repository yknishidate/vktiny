#pragma once
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>
#include <iostream>
#include <set>
#include "Window.hpp"

namespace vkt
{
    VKAPI_ATTR VkBool32 VKAPI_CALL debugUtilsMessengerCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageTypes,
        VkDebugUtilsMessengerCallbackDataEXT const* pCallbackData,
        void* /*pUserData*/);

    struct ContextCreateInfo
    {
        uint32_t apiMajorVersion = 1;
        uint32_t apiMinorVersion = 0;
        std::string appName = "";

        bool enableValidationLayer = false;
        std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
        vk::PhysicalDeviceFeatures features = {};
        void* deviceCreatePNext = nullptr; // TODO: managing this
    };

    class Context
    {
    public:
        Context(const ContextCreateInfo& info, const Window& window)
        {
            std::vector<const char*> layers = {};
            std::vector<const char*> instanceExtensions = window.getInstanceExtensions();
            if (info.enableValidationLayer) {
                layers.push_back("VK_LAYER_KHRONOS_validation");
                instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
            }

            initInstance(info.apiMajorVersion, info.apiMinorVersion,
                         info.appName, layers, instanceExtensions);
            if (info.enableValidationLayer) {
                initMessenger();
            }
            surface = window.createSurface(*instance);
            pickPhysicalDevice();
            findQueueFamilies();
            initDevice(info.deviceExtensions, info.features, info.deviceCreatePNext);
            getQueues();
            createCommandPools();
        }

        Context(const Context&) = delete;
        Context(Context&&) = default;
        Context& operator=(const Context&) = delete;
        Context& operator=(Context&&) = default;

        vk::UniqueCommandBuffer allocateGraphicsCommandBuffer() const
        {
            vk::CommandBufferAllocateInfo allocInfo;
            allocInfo.setCommandPool(*graphicsCommandPool);
            allocInfo.setCommandBufferCount(1);
            return std::move(device->allocateCommandBuffersUnique(allocInfo).front());
        }

        vk::UniqueCommandBuffer allocateComputeCommandBuffer() const
        {
            vk::CommandBufferAllocateInfo allocInfo;
            allocInfo.setCommandPool(*computeCommandPool);
            allocInfo.setCommandBufferCount(1);
            return std::move(device->allocateCommandBuffersUnique(allocInfo).front());
        }

        void submitGraphicsCommandBuffer(const vk::UniqueCommandBuffer& commandBuffer) const
        {
            vk::SubmitInfo submitInfo{ nullptr, nullptr, *commandBuffer };
            graphicsQueue.submit(submitInfo, nullptr);
            graphicsQueue.waitIdle();
        }

        void submitComputeCommandBuffer(const vk::UniqueCommandBuffer& commandBuffer) const
        {
            vk::SubmitInfo submitInfo{ nullptr, nullptr, *commandBuffer };
            computeQueue.submit(submitInfo, nullptr);
            computeQueue.waitIdle();
        }

        template <typename Func>
        void OneTimeSubmitGraphics(const Func& func) const
        {
            vk::UniqueCommandBuffer commandBuffer = allocateGraphicsCommandBuffer();

            commandBuffer->begin({ vk::CommandBufferUsageFlagBits::eOneTimeSubmit });
            func(*commandBuffer);
            commandBuffer->end();

            submitGraphicsCommandBuffer(commandBuffer);
        }

        template <typename Func>
        void OneTimeSubmitCompute(const Func& func) const
        {
            vk::UniqueCommandBuffer commandBuffer = allocateComputeCommandBuffer();

            commandBuffer->begin({ vk::CommandBufferUsageFlagBits::eOneTimeSubmit });
            func(*commandBuffer);
            commandBuffer->end();

            submitComputeCommandBuffer(commandBuffer);
        }

        uint32_t findMemoryType(uint32_t typeFilter,
                                vk::MemoryPropertyFlags properties) const
        {
            vk::PhysicalDeviceMemoryProperties memProperties = physicalDevice.getMemoryProperties();
            for (uint32_t i = 0; i != memProperties.memoryTypeCount; ++i) {
                if ((typeFilter & (1 << i)) &&
                    (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                    return i;
                }
            }
            throw std::runtime_error("failed to find suitable memory type");
        }

        vk::Device getDevice() const { return *device; }
        vk::PhysicalDevice getPhysicalDevice() const { return physicalDevice; }
        vk::SurfaceKHR getSurface() const { return *surface; }

        uint32_t getGraphicsFamily() const { return graphicsFamily; }
        uint32_t getComputeFamily() const { return computeFamily; }
        uint32_t getPresentFamily() const { return presentFamily; }

        vk::Queue getGraphicsQueue() const { return graphicsQueue; }
        vk::Queue getComputeQueue() const { return computeQueue; }
        vk::Queue getPresentQueue() const { return presentQueue; }

        vk::CommandPool getGraphicsCommandPool() const { return *graphicsCommandPool; }
        vk::CommandPool getComputeCommandPool() const { return *computeCommandPool; }

    private:
        void initInstance(uint32_t majorVersion,
                          uint32_t minorVersion,
                          const std::string& appName,
                          const std::vector<const char*>& layers,
                          const std::vector<const char*>& extensions)
        {
            vk::ApplicationInfo appInfo;
            appInfo.setApiVersion(VK_MAKE_API_VERSION(0, majorVersion, minorVersion, 0));
            appInfo.setPApplicationName(appName.c_str());

            static vk::DynamicLoader dl;
            auto vkGetInstanceProcAddr = dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
            VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);

            vk::InstanceCreateInfo instInfo;
            instInfo.setPApplicationInfo(&appInfo);
            instInfo.setPEnabledLayerNames(layers);
            instInfo.setPEnabledExtensionNames(extensions);
            instance = vk::createInstanceUnique(instInfo);

            VULKAN_HPP_DEFAULT_DISPATCHER.init(*instance);
        }

        void initMessenger()
        {
            using vkMS = vk::DebugUtilsMessageSeverityFlagBitsEXT;
            using vkMT = vk::DebugUtilsMessageTypeFlagBitsEXT;
            vk::DebugUtilsMessengerCreateInfoEXT messengerInfo;
            messengerInfo.setMessageSeverity(vkMS::eWarning | vkMS::eError);
            messengerInfo.setMessageType(vkMT::eGeneral | vkMT::ePerformance | vkMT::eValidation);
            messengerInfo.setPfnUserCallback(&debugUtilsMessengerCallback);
            messenger = instance->createDebugUtilsMessengerEXTUnique(messengerInfo);
        }

        void pickPhysicalDevice()
        {
            physicalDevice = instance->enumeratePhysicalDevices().front();
        }

        void findQueueFamilies()
        {
            auto queueFamilies = physicalDevice.getQueueFamilyProperties();
            for (int i = 0; i < queueFamilies.size(); i++) {
                const auto& queueFamily = queueFamilies[i];
                if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
                    graphicsFamily = i;
                }
                if (queueFamily.queueFlags & vk::QueueFlagBits::eCompute) {
                    computeFamily = i;
                }
                vk::Bool32 presentSupport = physicalDevice.getSurfaceSupportKHR(i, *surface);
                if (presentSupport) {
                    presentFamily = i;
                }
            }
        }

        void initDevice(const std::vector<const char*>& extensions,
                        vk::PhysicalDeviceFeatures features,
                        void* pNext)
        {
            std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
            std::set<uint32_t> uniqueQueueFamilies = { graphicsFamily, computeFamily, presentFamily };
            float queuePriority = 1.0f;
            for (uint32_t queueFamily : uniqueQueueFamilies) {
                vk::DeviceQueueCreateInfo queueCreateInfo{ {}, queueFamily, 1, &queuePriority };
                queueCreateInfos.push_back(queueCreateInfo);
            }

            vk::DeviceCreateInfo deviceInfo;
            deviceInfo.setQueueCreateInfos(queueCreateInfos);
            deviceInfo.setPEnabledExtensionNames(extensions);
            deviceInfo.setPEnabledFeatures(&features);
            deviceInfo.setPNext(pNext);
            device = physicalDevice.createDeviceUnique(deviceInfo);
        }

        void getQueues()
        {
            graphicsQueue = device->getQueue(graphicsFamily, 0);
            computeQueue = device->getQueue(computeFamily, 0);
            presentQueue = device->getQueue(presentFamily, 0);
        }

        void createCommandPools()
        {
            auto flag = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
            graphicsCommandPool = device->createCommandPoolUnique({ flag, graphicsFamily });
            computeCommandPool = device->createCommandPoolUnique({ flag, computeFamily });
        }

        vk::UniqueInstance instance;
        vk::UniqueDebugUtilsMessengerEXT messenger;
        vk::PhysicalDevice physicalDevice;
        vk::UniqueSurfaceKHR surface;
        vk::UniqueDevice device;

        uint32_t graphicsFamily = {};
        uint32_t presentFamily = {};
        uint32_t computeFamily = {};

        vk::Queue graphicsQueue;
        vk::Queue presentQueue;
        vk::Queue computeQueue;

        vk::UniqueCommandPool graphicsCommandPool;
        vk::UniqueCommandPool computeCommandPool;
    };
}
