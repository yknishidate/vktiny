#pragma once
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>
#include <utility>

namespace vkt
{
    class Window
    {
    public:
        Window() = default;
        Window(const Window&) = delete;
        Window(Window&&) = default;
        Window& operator=(const Window&) = delete;
        Window& operator=(Window&&) = default;

        void initialize(int width, int height, const std::string& title = {})
        {
            glfwInit();
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
            window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
        }

        vk::UniqueSurfaceKHR createSurface(vk::Instance instance) const
        {
            VkSurfaceKHR _surface;
            auto res = glfwCreateWindowSurface(VkInstance(instance), window, nullptr, &_surface);
            if (res != VK_SUCCESS) {
                throw std::runtime_error("failed to create window surface...");
            }
            return vk::UniqueSurfaceKHR{ vk::SurfaceKHR(_surface), {instance} };
        }

        std::vector<const char*> getInstanceExtensions() const
        {
            uint32_t glfwExtensionCount = 0;
            const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
            return std::vector<const char*>{glfwExtensions, glfwExtensions + glfwExtensionCount};
        }

        bool shouldClose() const
        {
            return glfwWindowShouldClose(window);
        }

        void pollEvents() const
        {
            glfwPollEvents();
        }

    private:
        GLFWwindow* window;
    };
}
