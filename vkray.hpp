
#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <set>
#include <optional>
#include <algorithm>
#include <fstream>
#include <vector>
#include <string>
#include <string_view>

#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>
VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace vkray
{

    class Context
    {
    public:
        void init(const bool enableValidation, const char* appName);
        bool shouldStop();
        void processEvents();
        ~Context();

        GLFWwindow* window;
        uint32_t width{ 1280 };
        uint32_t height{ 720 };

        vk::UniqueInstance instance;
        vk::PhysicalDevice physicalDevice;
        vk::UniqueDevice device;

        bool enableValidation;
        const char* appName;

    private:
        vk::UniqueDebugUtilsMessengerEXT debugUtilsMessenger;
        void createWindow();
        void createInstance();
        void createDebugMessenger();
        bool checkValidationLayerSupport();
        std::vector<const char*> getRequiredExtensions();

        static VKAPI_ATTR VkBool32 VKAPI_CALL debugUtilsMessengerCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT, VkDebugUtilsMessageTypeFlagsEXT,
            const VkDebugUtilsMessengerCallbackDataEXT*, void*);
    };


    void Context::init(const bool enableValidation, const char* appName)
    {
        this->enableValidation = enableValidation;
        this->appName = appName;

        createWindow();
        createInstance();
        createDebugMessenger();
    }

    inline bool Context::shouldStop()
    {
        return !glfwWindowShouldClose(window);
    }

    inline void Context::processEvents()
    {
        glfwPollEvents();
    }

    inline Context::~Context()
    {
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    void Context::createWindow()
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        window = glfwCreateWindow(width, height, appName, nullptr, nullptr);
    }

    void Context::createInstance()
    {
        // インスタンスに依存しない関数ポインタを取得する
        static vk::DynamicLoader dl;
        auto vkGetInstanceProcAddr = dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
        VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);

        if (enableValidation && !checkValidationLayerSupport()) {
            throw std::runtime_error("validation layers requested, but not available!");
        }

        vk::ApplicationInfo appInfo{
            appName,
            VK_MAKE_VERSION(1, 0, 0),
            "Engine",
            VK_MAKE_VERSION(1, 0, 0),
            VK_API_VERSION_1_2 };

        auto extensions = getRequiredExtensions();

        if (enableValidation) {
            // デバッグモードの場合
            vk::DebugUtilsMessageSeverityFlagsEXT severityFlags{
                vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning
                | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError };

            vk::DebugUtilsMessageTypeFlagsEXT messageTypeFlags{
                vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral
                | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance
                | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation };

            const std::vector<const char*> validationLayers = {
                "VK_LAYER_KHRONOS_validation"
            };
            vk::StructureChain<vk::InstanceCreateInfo, vk::DebugUtilsMessengerCreateInfoEXT> createInfo{
                { {}, &appInfo, validationLayers, extensions },
                { {}, severityFlags, messageTypeFlags, &debugUtilsMessengerCallback } };
            instance = vk::createInstanceUnique(createInfo.get<vk::InstanceCreateInfo>());
        } else {
            // リリースモードの場合
            vk::InstanceCreateInfo createInfo{ {}, &appInfo, {}, extensions };
            instance = vk::createInstanceUnique(createInfo, nullptr);
        }

        // 全ての関数ポインタを取得する
        VULKAN_HPP_DEFAULT_DISPATCHER.init(*instance);
    }


    void Context::createDebugMessenger()
    {
        if (!enableValidation) {
            return;
        }

        vk::DebugUtilsMessageSeverityFlagsEXT severityFlags{
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning
            | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError };
        vk::DebugUtilsMessageTypeFlagsEXT messageTypeFlags{
            vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral
            | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance
            | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation };

        vk::DebugUtilsMessengerCreateInfoEXT createInfo{
            {}, severityFlags, messageTypeFlags, &debugUtilsMessengerCallback };

        debugUtilsMessenger = instance->createDebugUtilsMessengerEXTUnique(createInfo);
    }


    VKAPI_ATTR VkBool32 VKAPI_CALL
        Context::debugUtilsMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageTypes,
            VkDebugUtilsMessengerCallbackDataEXT const* pCallbackData,
            void* /*pUserData*/)
    {
        std::cerr << "messageIDName   = " << pCallbackData->pMessageIdName << "\n";
        for (uint8_t i = 0; i < pCallbackData->objectCount; i++) {
            std::cerr << "objectType      = "
                << vk::to_string(static_cast<vk::ObjectType>(pCallbackData->pObjects[i].objectType)) << "\n";
        }
        std::cerr << pCallbackData->pMessage << "\n";
        std::cerr << "\n";
        return VK_FALSE;
    }

    bool Context::checkValidationLayerSupport()
    {
        std::vector<vk::LayerProperties> availableLayers = vk::enumerateInstanceLayerProperties();

        const char* layerName = "VK_LAYER_KHRONOS_validation";
        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                return true;
            }
        }
        return false;
    }

    std::vector<const char*> Context::getRequiredExtensions()
    {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        if (enableValidation) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return extensions;
    }

}
