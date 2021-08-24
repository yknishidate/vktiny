#include "vktiny/Vulkan/Context.hpp"
#include "vktiny/Log.hpp"
#include "stb_image.h"

namespace vkt
{
    Context::~Context()
    {
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    void Context::initialize(uint32_t apiVersion,
                             bool enableValidationLayer,
                             int width, int height,
                             std::vector<const char*> deviceExtensions,
                             vk::PhysicalDeviceFeatures features,
                             void* deviceCreatePNext)
    {
        log::set_pattern("[%^%-5l%$] %v");

        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        window = glfwCreateWindow(width, height, "Window", nullptr, nullptr);

        // instance extensions
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        std::vector<const char*> instanceExtensions(glfwExtensions,
                                                    glfwExtensions + glfwExtensionCount);

        // layers
        std::vector<const char*> layers;
        if (enableValidationLayer) {
            layers.push_back("VK_LAYER_KHRONOS_validation");
            instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        instance.initialize(apiVersion, layers, instanceExtensions);
        messenger.initialize(instance);
        surface.initialize(instance, window);
        physicalDevice.initialize(instance);
        device.initialize(instance, physicalDevice, surface, layers,
                          deviceExtensions, features, deviceCreatePNext);
        swapchain.initialize(device, physicalDevice, surface, width, height);
        input.initialize(window);
    }

    bool Context::running() const
    {
        return !glfwWindowShouldClose(window);
    }

    void Context::pollEvents()
    {
        input.reset();
        glfwPollEvents();
    }

    void Context::setWindowIcon(const std::string& filepath)
    {
        GLFWimage icon;
        icon.pixels = stbi_load(filepath.c_str(), &icon.width, &icon.height, nullptr, 4);
        if (icon.pixels != nullptr) {
            glfwSetWindowIcon(window, 1, &icon);
        }
        stbi_image_free(icon.pixels);
    }
}
