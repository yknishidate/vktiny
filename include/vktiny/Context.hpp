#include <vulkan/vulkan_raii.hpp>

struct ContextCreateInfo
{
    bool enableDebug = false;

    // Application
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

    void initialize(const ContextCreateInfo& info)
    {
        uint32_t version = VK_MAKE_API_VERSION(0, info.apiMajorVersion, info.apiMinorVersion, 0);

        vk::ApplicationInfo appInfo;
        appInfo.setApiVersion(version);

        vk::InstanceCreateInfo instInfo;
        instInfo.setPApplicationInfo(&appInfo);
        instInfo.setPEnabledLayerNames(info.instanceLayers);
        instInfo.setPEnabledExtensionNames(info.instanceExtensions);
        instance = vk::raii::Instance(context, instInfo);
    }

private:
    vk::raii::Context context;
    vk::raii::Instance instance = nullptr;
    vk::raii::PhysicalDevice physicalDevice = nullptr;
    vk::raii::Device device = nullptr;

    uint32_t graphicsFamily;
    uint32_t presentFamily;
    uint32_t computeFamily;

    vk::Queue graphicsQueue;
    vk::Queue presentQueue;
    vk::Queue computeQueue;
};
