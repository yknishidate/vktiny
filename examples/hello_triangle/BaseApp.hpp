#pragma once
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>
#include "Instance.hpp"
#include "DebugMessenger.hpp"
#include "Surface.hpp"
#include "PhysicalDevice.hpp"
#include "Device.hpp"
#include "Swapchain.hpp"
#include "ResourceManager.hpp"
#include "ShaderManager.hpp"
#include "Pipeline.hpp"
#include "Context.hpp"

class BaseApp
{
public:
    void run();

protected:
    int width = 1280;
    int height = 720;

    Context context;

    ResourceManager resourceManager;
    RayTracingShaderManager rtShaderManager;
    RayTracingPipeline rtPipeline;

    Image* renderImage;
    std::vector<vk::UniqueCommandBuffer> drawCommandBuffers;

private:
    void initVulkan();
    void prepare();
    void mainLoop();
    void draw();
};
