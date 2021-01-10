#pragma once

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <set>
#include <optional>
#include <algorithm>
#include <fstream>
#include <vector>
#include <string>

#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace vkf
{

namespace
{
#if defined(_DEBUG)
VKAPI_ATTR VkBool32 VKAPI_CALL debug_utils_messenger_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_type,
    const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
    void* user_data)
{
    std::cerr << "messageIDName   = " << callback_data->pMessageIdName << "\n";
    for (uint8_t i = 0; i < callback_data->objectCount; i++) {
        std::cerr << "objectType      = " << vk::to_string(static_cast<vk::ObjectType>(callback_data->pObjects[i].objectType)) << "\n";
    }
    std::cerr << callback_data->pMessage << "\n\n";
    return VK_FALSE;
}
#endif

bool validate_layers(const std::vector<const char*>& required, const std::vector<vk::LayerProperties>& available)
{
    for (auto layer : required) {
        bool found = false;
        for (auto& available_layer : available) {
            if (strcmp(available_layer.layerName, layer) == 0) {
                found = true;
                break;
            }
        }

        if (!found) {
            std::cout << "Validation Layer " << layer << " not found" << std::endl;
            return false;
        }
    }

    return true;
}
} // namespace

std::vector<const char*> get_optimal_validation_layers(const std::vector<vk::LayerProperties>& supported_instance_layers)
{
    std::vector<std::vector<const char*>> validation_layer_priority_list =
    {
        // The preferred validation layer is "VK_LAYER_KHRONOS_validation"
        {"VK_LAYER_KHRONOS_validation"},

        // Otherwise we fallback to using the LunarG meta layer
        {"VK_LAYER_LUNARG_standard_validation"},

        // Otherwise we attempt to enable the individual layers that compose the LunarG meta layer since it doesn't exist
        {
            "VK_LAYER_GOOGLE_threading",
            "VK_LAYER_LUNARG_parameter_validation",
            "VK_LAYER_LUNARG_object_tracker",
            "VK_LAYER_LUNARG_core_validation",
            "VK_LAYER_GOOGLE_unique_objects",
        },

        // Otherwise as a last resort we fallback to attempting to enable the LunarG core layer
        {"VK_LAYER_LUNARG_core_validation"} };

    for (auto& validation_layers : validation_layer_priority_list) {
        if (validate_layers(validation_layers, supported_instance_layers)) {
            return validation_layers;
        }
        std::cout << "Couldn't enable validation layers (see log for error) - falling back" << std::endl;
    }

    // Else return nothing
    return {};
}

class Instance
{

public:
    Instance(const std::string& application_name,
        const std::unordered_map<const char*, bool>& required_extensions = {},
        const std::vector<const char*>& required_validation_layers = {},
        uint32_t api_version = VK_API_VERSION_1_2)
    {
        auto available_instance_extensions = vk::enumerateInstanceExtensionProperties();

#if defined(_DEBUG)
        for (auto& available_extension : available_instance_extensions) {
            if (strcmp(available_extension.extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME) == 0) {
                std::cout << VK_EXT_DEBUG_UTILS_EXTENSION_NAME << " is available" << std::endl;
                enabled_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
            }
        }
#endif

        enabled_extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);

        for (auto& available_extension : available_instance_extensions) {
            // VK_KHR_get_physical_device_properties2 is a prerequisite of VK_KHR_performance_query
            // which will be used for stats gathering where available.
            if (strcmp(available_extension.extensionName, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME) == 0) {
                std::cout << VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME << " is available" << std::endl;
                enabled_extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
            }
        }

        auto extension_error = false;
        for (auto extension : required_extensions) {
            auto extension_name = extension.first;
            auto extension_is_optional = extension.second;
            if (std::find_if(available_instance_extensions.begin(), available_instance_extensions.end(),
                [&extension_name](vk::ExtensionProperties available_extension) { return strcmp(available_extension.extensionName, extension_name) == 0; }) == available_instance_extensions.end()) {
                if (extension_is_optional) {
                    std::cout << "Optional instance extension " << extension_name << " not available, some features may be disabled" << std::endl;
                } else {
                    std::cout << "Required instance extension " << extension_name << " not available, cannot run" << std::endl;
                }
                extension_error = !extension_is_optional;
            } else {
                enabled_extensions.push_back(extension_name);
            }
        }

        if (extension_error) {
            throw std::runtime_error("Required instance extensions are missing.");
        }

        auto supported_validation_layers = vk::enumerateInstanceLayerProperties();

        std::vector<const char*> requested_validation_layers(required_validation_layers);

#if defined(_DEBUG)
        // Determine the optimal validation layers to enable that are necessary for useful debugging
        std::vector<const char*> optimal_validation_layers = get_optimal_validation_layers(supported_validation_layers);
        requested_validation_layers.insert(requested_validation_layers.end(), optimal_validation_layers.begin(), optimal_validation_layers.end());
#endif

        if (validate_layers(requested_validation_layers, supported_validation_layers)) {
            std::cout << "Enabled Validation Layers:" << std::endl;
            for (const auto& layer : requested_validation_layers) {
                std::cout << "    " << layer << std::endl;
            }
        } else {
            throw std::runtime_error("Required validation layers are missing.");
        }

        vk::ApplicationInfo app_info{};
        app_info.pApplicationName = application_name.c_str();
        app_info.applicationVersion = 0;
        app_info.pEngineName = "Engine";
        app_info.engineVersion = 0;
        app_info.apiVersion = api_version;

        vk::InstanceCreateInfo instance_info{};
        instance_info.pApplicationInfo = &app_info;
        instance_info.enabledExtensionCount = static_cast<uint32_t>(enabled_extensions.size());
        instance_info.ppEnabledExtensionNames = enabled_extensions.data();
        instance_info.enabledLayerCount = static_cast<uint32_t>(requested_validation_layers.size());
        instance_info.ppEnabledLayerNames = requested_validation_layers.data();

#if defined(_DEBUG)
        vk::DebugUtilsMessengerCreateInfoEXT debug_utils_create_info{};
        debug_utils_create_info.messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eError | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning;
        debug_utils_create_info.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation;
        debug_utils_create_info.pfnUserCallback = debug_utils_messenger_callback;
        instance_info.pNext = &debug_utils_create_info;
#endif

        // Create the Vulkan instance
        handle = vk::createInstanceUnique(instance_info);
        VULKAN_HPP_DEFAULT_DISPATCHER.init(*handle);

#if defined(_DEBUG)
        debug_utils_messenger = handle->createDebugUtilsMessengerEXTUnique(debug_utils_create_info);
#endif

        query_gpus();
    }

    Instance(const Instance&) = delete;

    Instance(Instance&&) = delete;

    ~Instance();

    Instance& operator=(const Instance&) = delete;

    Instance& operator=(Instance&&) = delete;

    void query_gpus();

    PhysicalDevice& get_suitable_gpu();

    vk::Instance get_handle();

    const std::vector<const char*>& get_extensions();

private:
    vk::UniqueInstance handle;

    std::vector<const char*> enabled_extensions;

#if defined(_DEBUG)
    vk::UniqueDebugUtilsMessengerEXT debug_utils_messenger;
#endif

    std::vector<std::unique_ptr<PhysicalDevice>> gpus;
};
} // vkf
