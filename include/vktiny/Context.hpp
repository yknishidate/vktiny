#include <vulkan/vulkan_raii.hpp>
#include <GLFW/glfw3.h>
#include <iostream>

namespace vkt
{
    VKAPI_ATTR VkBool32 VKAPI_CALL debugUtilsMessengerCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageTypes,
        VkDebugUtilsMessengerCallbackDataEXT const* pCallbackData,
        void* /*pUserData*/)
    {
        std::cerr << pCallbackData->pMessage << "\n";
        return VK_FALSE;
    }

    struct ContextCreateInfo
    {
        void setDebug(bool enable = true)
        {
            enableDebug = enable;
            instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        void setWindowSize(int width, int height)
        {
            windowWidth = width;
            windowHeight = height;
        }

        void setAppName(const std::string& name)
        {
            appName = name;
        }

        void setApiVersion(uint32_t major, uint32_t minor)
        {
            apiMajorVersion = major;
            apiMinorVersion = minor;
        }

        void addInstanceLayer(const char* layer)
        {
            instanceLayers.push_back(layer);
        }

        void addInstanceExtension(const char* extension)
        {
            instanceExtensions.push_back(extension);
        }

        void addDeviceExtension(const char* extension)
        {
            deviceExtensions.push_back(extension);
        }

        void setFeaturesPNext(void* pNext)
        {
            featuresPNext = pNext;
        }

        // Debug
        bool enableDebug = false;

        // Window
        int windowWidth = 1280;
        int windowHeight = 720;

        // Application
        std::string appName = "Vulkan";
        uint32_t apiMajorVersion = 1;
        uint32_t apiMinorVersion = 0;

        // Instance
        std::vector<const char*> instanceLayers = {};
        std::vector<const char*> instanceExtensions = {};

        // Device
        std::vector<const char*> deviceExtensions = {};
        vk::PhysicalDeviceFeatures features;
        void* featuresPNext = nullptr; // TODO: managing this
    };

    class Context
    {
    public:
        Context() = default;
        Context(Context const&) = delete;
        Context& operator=(Context const&) = delete;

        void initialize(const ContextCreateInfo& info = {})
        {
            glfwInit();
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

            const int width = info.windowWidth;
            const int height = info.windowHeight;
            window = glfwCreateWindow(width, height, info.appName.c_str(), nullptr, nullptr);

            uint32_t glfwExtensionCount = 0;
            const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
            std::vector<const char*> instanceExtensions(glfwExtensions,
                                                        glfwExtensions + glfwExtensionCount);
            instanceExtensions.insert(instanceExtensions.begin(),
                                      info.instanceExtensions.begin(),
                                      info.instanceExtensions.end());

            uint32_t version = VK_MAKE_API_VERSION(0, info.apiMajorVersion, info.apiMinorVersion, 0);

            vk::ApplicationInfo appInfo;
            appInfo.setApiVersion(version);

            vk::InstanceCreateInfo instInfo;
            instInfo.setPApplicationInfo(&appInfo);
            instInfo.setPEnabledLayerNames(info.instanceLayers);
            instInfo.setPEnabledExtensionNames(instanceExtensions);
            instance = vk::raii::Instance(context, instInfo);

            if (info.enableDebug) {
                using MS = vk::DebugUtilsMessageSeverityFlagBitsEXT;
                using MT = vk::DebugUtilsMessageTypeFlagBitsEXT;
                vk::DebugUtilsMessengerCreateInfoEXT messengerInfo;
                messengerInfo.setMessageSeverity(MS::eWarning | MS::eError);
                messengerInfo.setMessageType(MT::eGeneral | MT::ePerformance | MT::eValidation);
                messengerInfo.setPfnUserCallback(&debugUtilsMessengerCallback);
                messenger = vk::raii::DebugUtilsMessengerEXT(instance, messengerInfo);
            }

            physicalDevice = std::move(vk::raii::PhysicalDevices(instance).front());

            VkSurfaceKHR _surface;
            auto res = glfwCreateWindowSurface(VkInstance(*instance), window, nullptr, &_surface);
            if (res != VK_SUCCESS) {
                throw std::runtime_error("failed to create window surface...");
            }
            surface = vk::raii::SurfaceKHR(instance, _surface);

            //int i = 0;
            //for (const auto& queueFamily : physicalDevice.getQueueFamilyProperties()) {
            //    if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
            //        graphicsFamily = i;
            //    }
            //    if (queueFamily.queueFlags & vk::QueueFlagBits::eCompute) {
            //        computeFamily = i;
            //    }
            //    vk::Bool32 presentSupport = physicalDevice.getSurfaceSupportKHR(i, surface);
            //    if (presentSupport) {
            //        presentFamily = i;
            //    }
            //    if (graphicsFamily != -1 && presentFamily != -1 && computeFamily != -1) {
            //        break;
            //    }
            //    i++;
            //}
        }

        bool shouldTerminate()
        {
            return glfwWindowShouldClose(window);
        }

        void pollEvents()
        {
            glfwPollEvents();
        }

    private:
        GLFWwindow* window;

        vk::raii::Context context;
        vk::raii::Instance instance = nullptr;
        vk::raii::DebugUtilsMessengerEXT messenger = nullptr;
        vk::raii::PhysicalDevice physicalDevice = nullptr;
        vk::raii::SurfaceKHR surface = nullptr;
        vk::raii::Device device = nullptr;

        uint32_t graphicsFamily = {};
        uint32_t presentFamily = {};
        uint32_t computeFamily = {};

        vk::Queue graphicsQueue;
        vk::Queue presentQueue;
        vk::Queue computeQueue;
    };
}
