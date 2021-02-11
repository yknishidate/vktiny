
#define QUICK_VKRAY_IMPLEMENTATION
#include "../../vkray.hpp"
#include <GLFW/glfw3.h>

using vkss = vk::ShaderStageFlagBits;
using vkdt = vk::DescriptorType;
using vksgt = vk::RayTracingShaderGroupTypeKHR;

constexpr int WIDTH = 800;
constexpr int HEIGHT = 600;

class Application
{
public:

    ~Application()
    {
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    void run()
    {
        initWindow();
        initVulkan();
        mainLoop();
    }

private:

    GLFWwindow* window;

    std::unique_ptr<vkr::Instance> instance;

    vk::UniqueSurfaceKHR surface;

    std::unique_ptr<vkr::Device> device;

    std::unique_ptr<vkr::SwapChain> swapChain;

    std::unique_ptr<vkr::Image> storageImage;

    std::unique_ptr<vkr::BottomLevelAccelerationStructure> blas;
    std::unique_ptr<vkr::TopLevelAccelerationStructure> tlas;

    std::unique_ptr<vkr::ShaderManager> shaderManager;

    std::unique_ptr<vkr::DescriptorSets> descSets;

    vk::UniquePipeline pipeline;

    vkr::Model model;

    void initWindow()
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        window = glfwCreateWindow(WIDTH, HEIGHT, "quick vkray", nullptr, nullptr);
    }

    void initVulkan()
    {
        createInstance();
        createSurface();
        device = std::make_unique<vkr::Device>(*instance, *surface);
        swapChain = std::make_unique<vkr::SwapChain>(*device, vk::Extent2D{ WIDTH, HEIGHT });

        storageImage = swapChain->createStorageImage();

        buildAccelStruct();

        loadShaders();

        createDescSets();

        pipeline = descSets->createRayTracingPipeline(*shaderManager, 1);
        shaderManager->initShaderBindingTable(*pipeline, 1, 1, 1);
        swapChain->initDrawCommandBuffers(*pipeline, *descSets, *shaderManager, *storageImage);
    }

    void createInstance()
    {
        // Get GLFW extensions
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

#ifdef _DEBUG
        bool enableValidationLayers = true;
#else
        enableValidationLayers = false;
#endif

        vk::ApplicationInfo appInfo;
        appInfo.setPApplicationName("quick vkray");
        appInfo.setApiVersion(VK_API_VERSION_1_2);

        instance = std::make_unique<vkr::Instance>(appInfo, enableValidationLayers, extensions);
    }

    void createSurface()
    {
        VkSurfaceKHR _surface;
        if (glfwCreateWindowSurface(VkInstance(instance->getHandle()), window, nullptr, &_surface) != VK_SUCCESS) {
            throw std::runtime_error("failed to create window surface!");
        }
        vk::ObjectDestroy<vk::Instance, VULKAN_HPP_DEFAULT_DISPATCHER_TYPE> _deleter(instance->getHandle());
        surface = vk::UniqueSurfaceKHR(vk::SurfaceKHR(_surface), _deleter);
    }

    void buildAccelStruct()
    {
        model.loadFromFile(*device, "samples/assets/Cube/Cube.gltf");
        const vkr::Mesh& mesh = model.getMeshes()[0];

        blas = std::make_unique<vkr::BottomLevelAccelerationStructure>(*device, mesh);

        vkr::AccelerationStructureInstance instance{ 0, glm::mat4(1) };

        tlas = std::make_unique<vkr::TopLevelAccelerationStructure>(*device, *blas, instance);
    }

    void loadShaders()
    {
        shaderManager = std::make_unique<vkr::ShaderManager>(*device);
        shaderManager->addShader("samples/01_gltf_mesh_texture/raygen.rgen.spv", vkss::eRaygenKHR, "main", vksgt::eGeneral);
        shaderManager->addShader("samples/01_gltf_mesh_texture/miss.rmiss.spv", vkss::eMissKHR, "main", vksgt::eGeneral);
        shaderManager->addShader("samples/01_gltf_mesh_texture/closesthit.rchit.spv", vkss::eClosestHitKHR, "main", vksgt::eTrianglesHitGroup);
    }

    void createDescSets()
    {
        const vkr::Mesh& mesh = model.getMeshes()[0];
        const vkr::Material& material = model.getMaterials()[mesh.materialIndex];
        const vkr::Texture& texture = model.getTextures()[material.baseColorTextureIndex];

        descSets = std::make_unique<vkr::DescriptorSets>(*device, 1);
        descSets->addBindging(0, 0, vkdt::eAccelerationStructureKHR, 1, vkss::eRaygenKHR); // TLAS
        descSets->addBindging(0, 1, vkdt::eStorageImage, 1, vkss::eRaygenKHR);             // Storage Image
        descSets->addBindging(0, 2, vkdt::eStorageBuffer, 1, vkss::eClosestHitKHR);        // Vertex
        descSets->addBindging(0, 3, vkdt::eStorageBuffer, 1, vkss::eClosestHitKHR);        // Index
        descSets->addBindging(0, 4, vkdt::eCombinedImageSampler, 1, vkss::eClosestHitKHR); // Texture
        descSets->initPipelineLayout();

        descSets->allocate();
        descSets->addWriteInfo(0, 0, tlas->createWrite());
        descSets->addWriteInfo(0, 1, storageImage->createDescriptorInfo());
        descSets->addWriteInfo(0, 2, mesh.vertexBuffer->createDescriptorInfo());
        descSets->addWriteInfo(0, 3, mesh.indexBuffer->createDescriptorInfo());
        descSets->addWriteInfo(0, 4, texture.createDescriptorInfo());
        descSets->update();
    }

    void mainLoop()
    {
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
            swapChain->draw();
        }
        device->waitIdle();
    }
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
