#include "vktiny/Vulkan/DebugMessenger.hpp"
#include "vktiny/Log.hpp"
#include <iostream>
#include <sstream>

VKAPI_ATTR VkBool32 VKAPI_CALL debugUtilsMessengerCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageTypes,
    VkDebugUtilsMessengerCallbackDataEXT const* pCallbackData,
    void* /*pUserData*/)
{
    std::stringstream ss;
    ss << "DebugUtilsMessage: ";
    if (messageTypes & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT) {
        ss << "General\n";
    } else if (messageTypes & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) {
        ss << "Validation\n";
    } else {
        ss << "Performance\n";
    }

    ss << "MessageID: " << pCallbackData->pMessageIdName << "\n";

    // Validation Error...
    std::string str = pCallbackData->pMessage;
    //ss << str;
    std::size_t next = str.find("Object ");
    ss << str.substr(0, next) << std::endl;
    str = str.substr(next);

    // (cut)
    next = str.find("|") + 2;
    str = str.substr(next);
    next = str.find("|") + 2;
    str = str.substr(next);

    // Message
    next = str.find("The Vulkan spec");
    if (next != std::string::npos) {
        ss << str.substr(0, next) << std::endl;
        str = str.substr(next);
    }
    ss << str << std::endl;

    if ((messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)) {
        vkt::log::error(ss.str());
    } else if ((messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)) {
        vkt::log::warn(ss.str());
    } else {
        vkt::log::info(ss.str());
    }

    return VK_FALSE;
}

namespace vkt
{
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
}
