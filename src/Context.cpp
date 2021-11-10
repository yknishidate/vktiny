#include "..\include\vktiny\Context.hpp"

namespace vkt
{
    VKAPI_ATTR VkBool32 VKAPI_CALL debugUtilsMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageTypes, VkDebugUtilsMessengerCallbackDataEXT const* pCallbackData, void*)
    {
        std::cerr << pCallbackData->pMessage << "\n";
        return VK_FALSE;
    }
}
