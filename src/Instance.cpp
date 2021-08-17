#include <iostream>
#include "vktiny/Instance.hpp"

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

void Instance::initialize(uint32_t apiVersion,
                          std::vector<const char*> layers,
                          std::vector<const char*> extensions)
{
    static vk::DynamicLoader dl;
    auto vkGetInstanceProcAddr = dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
    VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);

    vk::ApplicationInfo appInfo;
    appInfo.setApiVersion(apiVersion);

    instance = vk::createInstanceUnique({ {}, &appInfo, layers, extensions });
    VULKAN_HPP_DEFAULT_DISPATCHER.init(*instance);
}
