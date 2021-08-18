#include <iostream>
#include "vktiny/Surface.hpp"

namespace vkt
{
    void Surface::initialize(const Instance& instance, GLFWwindow* window)
    {
        VkSurfaceKHR _surface;
        auto res = glfwCreateWindowSurface(VkInstance(instance.get()), window, nullptr, &_surface);
        if (res != VK_SUCCESS) {
            throw std::runtime_error("failed to create window surface!");
        }
        surface = vk::UniqueSurfaceKHR(vk::SurfaceKHR(_surface), { instance.get() });
    }
}
