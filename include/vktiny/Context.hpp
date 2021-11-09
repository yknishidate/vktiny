#include <vulkan/vulkan_raii.hpp>
#include <GLFW/glfw3.h>
#include <iostream>
#include <set>

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
        friend class Context;

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

    private:
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
            initWindow(info.windowWidth, info.windowHeight, info.appName);

            auto layer = info.instanceLayers;
            auto extensions = info.instanceExtensions;
            auto glfwExtensions = getGLFWExtension();
            extensions.insert(extensions.begin(), glfwExtensions.begin(), glfwExtensions.end());
            if (info.enableDebug) {
                layer.push_back("VK_LAYER_KHRONOS_validation");
                extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
            }
            initInstance(info.apiMajorVersion, info.apiMinorVersion, info.appName,
                         layer, extensions);

            if (info.enableDebug) {
                initMessenger();
            }

            physicalDevice = std::move(vk::raii::PhysicalDevices(instance).front());

            initSurface();

            findQueueFamilies();

            initDevice(info.deviceExtensions, info.features, info.featuresPNext);
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
        void initWindow(int width, int height, const std::string& title)
        {
            glfwInit();
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
            window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
        }

        std::vector<const char*> getGLFWExtension()
        {
            uint32_t count = 0;
            const char** extensions = glfwGetRequiredInstanceExtensions(&count);
            return std::vector<const char*>(extensions, extensions + count);
        }

        void initInstance(uint32_t majorVersion,
                          uint32_t minorVersion,
                          const std::string& appName,
                          const std::vector<const char*>& layers,
                          const std::vector<const char*>& extensions)
        {
            vk::ApplicationInfo appInfo;
            appInfo.setApiVersion(VK_MAKE_API_VERSION(0, majorVersion, minorVersion, 0));
            appInfo.setPApplicationName(appName.c_str());

            vk::InstanceCreateInfo instInfo;
            instInfo.setPApplicationInfo(&appInfo);
            instInfo.setPEnabledLayerNames(layers);
            instInfo.setPEnabledExtensionNames(extensions);
            instance = vk::raii::Instance(context, instInfo);
        }

        void initMessenger()
        {
            using MS = vk::DebugUtilsMessageSeverityFlagBitsEXT;
            using MT = vk::DebugUtilsMessageTypeFlagBitsEXT;
            vk::DebugUtilsMessengerCreateInfoEXT messengerInfo;
            messengerInfo.setMessageSeverity(MS::eWarning | MS::eError);
            messengerInfo.setMessageType(MT::eGeneral | MT::ePerformance | MT::eValidation);
            messengerInfo.setPfnUserCallback(&debugUtilsMessengerCallback);
            messenger = vk::raii::DebugUtilsMessengerEXT(instance, messengerInfo);
        }

        void initSurface()
        {
            VkSurfaceKHR _surface;
            auto res = glfwCreateWindowSurface(VkInstance(*instance), window, nullptr, &_surface);
            if (res != VK_SUCCESS) {
                throw std::runtime_error("failed to create window surface...");
            }
            surface = vk::raii::SurfaceKHR(instance, _surface);
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
            device = vk::raii::Device(physicalDevice, deviceInfo);
        }

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
