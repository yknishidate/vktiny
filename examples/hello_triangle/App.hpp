#pragma once
#include "ResourceManager.hpp"
#include "ShaderManager.hpp"
#include "Pipeline.hpp"
#include "Context.hpp"
#include "Mesh.hpp"

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
    std::vector<vk::UniqueCommandBuffer> drawCommandBuffers;

    using Index = uint32_t;
    Image* renderImage;
    Buffer* vertexBuffer;
    Buffer* indexBuffer;
    std::vector<Vertex> vertices;
    std::vector<Index> indices;
};
