#pragma once

#include <set>
#include <map>
#include <vector>
#include <string>
#include <fstream>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <algorithm>

#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>

#include <GLFW/glfw3.h>

#define STBI_NO_BMP
#define STBI_NO_PSD
#define STBI_NO_TGA
#define STBI_NO_GIF
#define STBI_NO_HDR
#define STBI_NO_PIC
#define STBI_NO_PNM
#include <stb_image.h>

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace vkray
{

    class Window final
    {
    public:
        explicit Window(const std::string& title, const uint32_t width, const uint32_t height,
            bool cursorDisabled = false, bool fullscreen = false, bool resizable = false);

        ~Window()
        {
            if (window != nullptr) {
                glfwDestroyWindow(window);
                window = nullptr;
            }

            glfwTerminate();
            glfwSetErrorCallback(nullptr);
        }

        Window(const Window&) = delete;
        Window(Window&&) = delete;
        Window& operator = (const Window&) = delete;
        Window& operator = (Window&&) = delete;

        GLFWwindow* getHandle() const { return window; }

        float getContentScale() const
        {
            float xscale;
            float yscale;
            glfwGetWindowContentScale(window, &xscale, &yscale);

            return xscale;
        }

        vk::Extent2D getFramebufferSize() const
        {
            int width, height;
            glfwGetFramebufferSize(window, &width, &height);
            return vk::Extent2D{ static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
        }

        vk::Extent2D getWindowSize() const
        {
            int width, height;
            glfwGetWindowSize(window, &width, &height);
            return vk::Extent2D{ static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
        }

        const char* getKeyName(int key, int scancode) const { return glfwGetKeyName(key, scancode); }

        std::vector<const char*> getRequiredInstanceExtensions() const
        {
            uint32_t glfwExtensionCount = 0;
            const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
            return std::vector<const char*>(glfwExtensions, glfwExtensions + glfwExtensionCount);
        }

        double getTime() const { return glfwGetTime(); }

        std::string getTitle() const { return title; }

        // Callbacks
        std::function<void()> drawFrame;
        std::function<void(int key, int scancode, int action, int mods)> onKey;
        std::function<void(double xpos, double ypos)> onCursorPosition;
        std::function<void(int button, int action, int mods)> onMouseButton;
        std::function<void(double xoffset, double yoffset)> onScroll;

        // Methods
        void close() { glfwSetWindowShouldClose(window, 1); }

        bool isMinimized() const
        {
            const auto size = getFramebufferSize();
            return size.height == 0 && size.width == 0;
        }

        void run()
        {
            glfwSetTime(0.0);

            while (!glfwWindowShouldClose(window)) {
                glfwPollEvents();

                if (drawFrame) {
                    drawFrame();
                }
            }
        }

        void waitForEvents() const { glfwWaitEvents(); }

    private:

        GLFWwindow* window{};
        std::string title;
        uint32_t width;
        uint32_t height;
        bool cursorDisabled;
        bool fullscreen;
        bool resizable;
    };

    class Instance final
    {
    public:
        Instance(const Window& window, const bool enableValidationLayers);
        ~Instance() {}

        Instance(const Instance&) = delete;
        Instance(Instance&&) = delete;
        Instance& operator = (const Instance&) = delete;
        Instance& operator = (Instance&&) = delete;

        vk::Instance getHandle() const { return *instance; }
        const class Window& getWindow() const { return window; }

        const std::vector<vk::ExtensionProperties>& getExtensions() const { return extensions; }
        const std::vector<vk::PhysicalDevice>& getPhysicalDevices() const { return physicalDevices; }
        const std::vector<const char*>& getValidationLayers() const { return validationLayers; }

    private:
        void getVulkanDevices()
        {
            physicalDevices = instance->enumeratePhysicalDevices();

            if (physicalDevices.empty()) {
                throw std::runtime_error("found no Vulkan physical devices");
            }
        }

        void getVulkanExtensions()
        {
            extensions = vk::enumerateInstanceExtensionProperties();
        }

        void createDebugMessenger();

        static void checkVulkanMinimumVersion(uint32_t minVersion)
        {
            uint32_t version = vk::enumerateInstanceVersion();
            if (minVersion > version) {
                throw std::runtime_error("minimum required version not found");
            }
        }

        static void checkVulkanValidationLayerSupport(const std::vector<const char*>& validationLayers)
        {
            const auto availableLayers = vk::enumerateInstanceLayerProperties();

            for (const char* layer : validationLayers) {
                auto result = std::find_if(availableLayers.begin(), availableLayers.end(), [layer](const VkLayerProperties& layerProperties) {
                    return strcmp(layer, layerProperties.layerName) == 0;
                });

                if (result == availableLayers.end()) {
                    throw std::runtime_error("could not find the requested validation layer: '" + std::string(layer) + "'");
                }
            }
        }

        vk::UniqueInstance instance{};
        const class Window& window;
        bool enableValidationLayers;
        std::vector<const char*> validationLayers;

        std::vector<vk::PhysicalDevice> physicalDevices;
        std::vector<vk::ExtensionProperties> extensions;

        vk::UniqueDebugUtilsMessengerEXT messenger;

    };


    namespace
    {
#if defined(_DEBUG)
        VKAPI_ATTR VkBool32 VKAPI_CALL
            debugUtilsMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                VkDebugUtilsMessengerCallbackDataEXT const* pCallbackData, void* /*pUserData*/)
        {
            std::cerr << "messageIDName   = " << pCallbackData->pMessageIdName << "\n";

            for (uint8_t i = 0; i < pCallbackData->objectCount; i++) {
                std::cerr << "objectType      = " << vk::to_string(static_cast<vk::ObjectType>(pCallbackData->pObjects[i].objectType)) << "\n";
            }

            std::cerr << pCallbackData->pMessage << "\n\n";

            return VK_FALSE;
        }
#endif

        void glfwErrorCallback(const int error, const char* const description)
        {
            std::cerr << "ERROR: GLFW: " << description << " (code: " << error << ")" << std::endl;
        }

        void glfwKeyCallback(GLFWwindow* window, const int key, const int scancode, const int action, const int mods)
        {
            auto* const this_ = static_cast<Window*>(glfwGetWindowUserPointer(window));
            if (this_->onKey) {
                this_->onKey(key, scancode, action, mods);
            }
        }

        void glfwCursorPositionCallback(GLFWwindow* window, const double xpos, const double ypos)
        {
            auto* const this_ = static_cast<Window*>(glfwGetWindowUserPointer(window));
            if (this_->onCursorPosition) {
                this_->onCursorPosition(xpos, ypos);
            }
        }

        void glfwMouseButtonCallback(GLFWwindow* window, const int button, const int action, const int mods)
        {
            auto* const this_ = static_cast<Window*>(glfwGetWindowUserPointer(window));
            if (this_->onMouseButton) {
                this_->onMouseButton(button, action, mods);
            }
        }

        void glfwScrollCallback(GLFWwindow* window, const double xoffset, const double yoffset)
        {
            auto* const this_ = static_cast<Window*>(glfwGetWindowUserPointer(window));
            if (this_->onScroll) {
                this_->onScroll(xoffset, yoffset);
            }
        }

    } // namespace

    ////////////////////
    // implementation //
    ////////////////////

    Window::Window(const std::string& title, const uint32_t width, const uint32_t height, bool cursorDisabled, bool fullscreen, bool resizable)
        : title(title), width(width), height(height), cursorDisabled(cursorDisabled), fullscreen(fullscreen), resizable(resizable)
    {
        glfwSetErrorCallback(glfwErrorCallback);

        if (!glfwInit()) {
            throw std::runtime_error("glfwInit() failed");
        }

        if (!glfwVulkanSupported()) {
            throw std::runtime_error("glfwVulkanSupported() failed");
        }

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, resizable ? GLFW_TRUE : GLFW_FALSE);

        auto* const monitor = fullscreen ? glfwGetPrimaryMonitor() : nullptr;

        window = glfwCreateWindow(width, height, title.c_str(), monitor, nullptr);
        if (window == nullptr) {
            throw std::runtime_error("failed to create window");
        }

        if (cursorDisabled) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }

        glfwSetWindowUserPointer(window, this);
        glfwSetKeyCallback(window, glfwKeyCallback);
        glfwSetCursorPosCallback(window, glfwCursorPositionCallback);
        glfwSetMouseButtonCallback(window, glfwMouseButtonCallback);
        glfwSetScrollCallback(window, glfwScrollCallback);
    }

    Instance::Instance(const Window& window, const bool enableValidationLayers)
        : window(window)
        , enableValidationLayers(enableValidationLayers)
    {
        static vk::DynamicLoader dl;
        auto vkGetInstanceProcAddr = dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
        VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);

        validationLayers = enableValidationLayers ? std::vector<const char*>{"VK_LAYER_KHRONOS_validation"} : std::vector<const char*>();

        const uint32_t version = VK_API_VERSION_1_2;

        checkVulkanMinimumVersion(version);

        auto extensions = window.getRequiredInstanceExtensions();

        checkVulkanValidationLayerSupport(validationLayers);

        if (enableValidationLayers) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        vk::ApplicationInfo appInfo{};
        appInfo.pApplicationName = window.getTitle().c_str();
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_2;

        vk::InstanceCreateInfo createInfo{};
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();

        instance = vk::createInstanceUnique(createInfo);

        VULKAN_HPP_DEFAULT_DISPATCHER.init(*instance);

        getVulkanDevices();
        getVulkanExtensions();
        if (enableValidationLayers) {
            createDebugMessenger();
        }
    }

    void Instance::createDebugMessenger()
    {
        vk::DebugUtilsMessageSeverityFlagsEXT severityFlags{
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning
            | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError };

        vk::DebugUtilsMessageTypeFlagsEXT messageTypeFlags{
            vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral
            | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance
            | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation };

        vk::DebugUtilsMessengerCreateInfoEXT createInfo{ {}, severityFlags, messageTypeFlags, &debugUtilsMessengerCallback };

        messenger = instance->createDebugUtilsMessengerEXTUnique(createInfo);
    }

} // vkf
