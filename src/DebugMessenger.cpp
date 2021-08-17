#include "DebugMessenger.hpp"
#include <iostream>

VKAPI_ATTR VkBool32 VKAPI_CALL debugUtilsMessengerCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageTypes,
    VkDebugUtilsMessengerCallbackDataEXT const* pCallbackData,
    void* /*pUserData*/)
{
    std::cerr << "messageIndexName   = " << pCallbackData->pMessageIdName << "\n";
    for (uint8_t i = 0; i < pCallbackData->objectCount; i++) {
        std::cerr << "objectType      = " << vk::to_string(static_cast<vk::ObjectType>(pCallbackData->pObjects[i].objectType)) << "\n";
    }
    std::cerr << pCallbackData->pMessage << "\n\n";
    return VK_FALSE;
}

void DebugMessenger::initialize(const Instance& instance)
{
    using vkMS = vk::DebugUtilsMessageSeverityFlagBitsEXT;
    using vkMT = vk::DebugUtilsMessageTypeFlagBitsEXT;

    vk::DebugUtilsMessengerCreateInfoEXT createInfo;
    createInfo.setMessageSeverity(vkMS::eWarning | vkMS::eError);
    createInfo.setMessageType(vkMT::eGeneral | vkMT::ePerformance | vkMT::eValidation);
    createInfo.setPfnUserCallback(&debugUtilsMessengerCallback);
    messenger = instance.get().createDebugUtilsMessengerEXTUnique(createInfo);
}
