#pragma once
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>
#include "Instance.hpp"

class Surface
{
public:
    void initialize(const Instance& instance, GLFWwindow* window);

    vk::SurfaceKHR get() const { return surface.get(); }

private:
    vk::UniqueSurfaceKHR surface;
};
