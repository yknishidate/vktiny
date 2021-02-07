
#define QUICK_VKRAY_IMPLEMENTATION
#include "../../vkray.hpp"

using vkss = vk::ShaderStageFlagBits;
using vkdt = vk::DescriptorType;
using vksgt = vk::RayTracingShaderGroupTypeKHR;

class Application
{
public:

    void run()
    {
        window = std::make_unique<vkr::Window>("vkray", 800, 600);
        instance = std::make_unique<vkr::Instance>(*window, true);
        device = std::make_unique<vkr::Device>(*instance);
        swapChain = std::make_unique<vkr::SwapChain>(*device, *window);

        // Create storage image
        storageImage = swapChain->createStorageImage();

        // Create Mesh
        std::vector<vkr::Vertex> vertices{
            { {1.0f, 1.0f, 0.0f} },
            { {-1.0f, 1.0f, 0.0f} },
            { {0.0f, -1.0f, 0.0f} } };
        std::vector<uint32_t> indices{ 0, 1, 2 };
        vkr::Mesh mesh;
        mesh.create(*device, vertices, indices);

        // Create BLAS
        blas = std::make_unique<vkr::BottomLevelAccelerationStructure>(*device, mesh);

        // Create TLAS
        vkr::AccelerationStructureInstance instance{ 0, glm::mat4(1) };
        tlas = std::make_unique<vkr::TopLevelAccelerationStructure>(*device, *blas, instance);

        // Load shaders
        shaderManager = std::make_unique<vkr::ShaderManager>(*device);
        shaderManager->addShader("samples/00_hello_triangle/raygen.rgen.spv", vkss::eRaygenKHR, "main", vksgt::eGeneral);
        shaderManager->addShader("samples/00_hello_triangle/miss.rmiss.spv", vkss::eMissKHR, "main", vksgt::eGeneral);
        shaderManager->addShader("samples/00_hello_triangle/closesthit.rchit.spv", vkss::eClosestHitKHR, "main", vksgt::eTrianglesHitGroup);

        // Create Desc Sets
        descSets = std::make_unique<vkr::DescriptorSets>(*device, 1);
        descSets->addBindging(0, 0, vkdt::eAccelerationStructureKHR, 1, vkss::eRaygenKHR);
        descSets->addBindging(0, 1, vkdt::eStorageImage, 1, vkss::eRaygenKHR);
        descSets->initPipelineLayout();

        descSets->allocate();
        descSets->addWriteInfo(0, 0, tlas->createWrite());
        descSets->addWriteInfo(0, 1, storageImage->createDescriptorInfo());
        descSets->update();

        // Create Ray Tracing Pipeline
        pipeline = device->createRayTracingPipeline(*descSets, *shaderManager, 1);

        // Init Shader Binding Table
        shaderManager->initShaderBindingTable(*pipeline, 0, 1, 2);

        // Init Draw Command Buffers
        swapChain->initDrawCommandBuffers(*pipeline, *descSets, *shaderManager, *storageImage);

        // Main loop
        while (!window->shouldClose()) {
            window->pollEvents();
            swapChain->draw();
        }

        device->waitIdle();

    }

private:

    std::unique_ptr<vkr::Window> window;
    std::unique_ptr<vkr::Instance> instance;
    std::unique_ptr<vkr::Device> device;
    std::unique_ptr<vkr::SwapChain> swapChain;

    std::unique_ptr<vkr::Image> storageImage;
    std::unique_ptr<vkr::BottomLevelAccelerationStructure> blas;
    std::unique_ptr<vkr::TopLevelAccelerationStructure> tlas;

    std::unique_ptr<vkr::DescriptorSets> descSets;
    std::unique_ptr<vkr::ShaderManager> shaderManager;

    vk::UniquePipeline pipeline;

};

int main()
{
    Application app;

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
