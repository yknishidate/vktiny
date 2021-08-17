#pragma once
#include "ResourceManager.hpp"
#include "ShaderManager.hpp"
#include "Pipeline.hpp"
#include "Context.hpp"

class App
{
public:
    void run();

private:
    void initVulkan();
    void prepare();
    void mainLoop();
    void draw();

    int width = 1280;
    int height = 720;
    Context context;

    ResourceManager resourceManager;
    RayTracingShaderManager rtShaderManager;
    RayTracingPipeline rtPipeline;

    Image* renderImage;
    std::vector<vk::UniqueCommandBuffer> drawCommandBuffers;
};
