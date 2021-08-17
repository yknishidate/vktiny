#include "Context.hpp"

Context::~Context()
{
    glfwDestroyWindow(window);
    glfwTerminate();
}

void Context::initialize(uint32_t apiVersion,
                         bool enableValidationLayer,
                         int width, int height,
                         std::vector<const char*> deviceExtensions,
                         void* deviceCreatePNext)
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    window = glfwCreateWindow(width, height, "Window", nullptr, nullptr);

    // instance extensions
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector<const char*> instanceExtensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    // layers
    std::vector<const char*> layers;
    if (enableValidationLayer) {
        layers.push_back("VK_LAYER_KHRONOS_validation");
        instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    instance.initialize(VK_API_VERSION_1_2, layers, instanceExtensions);
    messenger.initialize(instance);
    surface.initialize(instance, window);
    physicalDevice.initialize(instance);
    device.initialize(instance, physicalDevice, surface, layers,
                      deviceExtensions, deviceCreatePNext);
    swapchain.initialize(device, physicalDevice, surface, width, height);
}

bool Context::running() const
{
    return !glfwWindowShouldClose(window);
}

void Context::pollEvents() const
{
    glfwPollEvents();
}
