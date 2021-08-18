#pragma once
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>
#include "vktiny/Vulkan/Instance.hpp"

namespace vkt
{
    class Surface
    {
    public:
        Surface() = default;
        Surface(const Surface&) = delete;
        Surface(Surface&&) = default;
        Surface& operator = (const Surface&) = delete;
        Surface& operator = (Surface&&) = default;

        void initialize(const Instance& instance, GLFWwindow* window);

        vk::SurfaceKHR get() const { return surface.get(); }

    private:
        vk::UniqueSurfaceKHR surface;
    };
}
