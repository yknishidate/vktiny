
#define QUICK_VKRAY_IMPLEMENTATION
#include "../../vkray.hpp"
#include <GLFW/glfw3.h>

constexpr int WIDTH = 800;
constexpr int HEIGHT = 600;

int main()
{
#ifdef _DEBUG
    bool enableValidationLayers = true;
#else
    enableValidationLayers = false;
#endif

    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    auto window = glfwCreateWindow(WIDTH, HEIGHT, "quick vkray", nullptr, nullptr);

    // Get GLFW extensions
    uint32_t glfwExtensionCount = 0;
    auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    // Create instance
    vk::ApplicationInfo appInfo;
    appInfo.setPApplicationName("quick vkray");
    appInfo.setApiVersion(VK_API_VERSION_1_2);

    vkr::Instance instance{ appInfo, enableValidationLayers, extensions };

    // Create surface
    VkSurfaceKHR _surface;
    if (glfwCreateWindowSurface(VkInstance(instance.getHandle()), window, nullptr, &_surface) != VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface!");
    }
    vk::ObjectDestroy<vk::Instance, VULKAN_HPP_DEFAULT_DISPATCHER_TYPE> _deleter(instance.getHandle());
    vk::UniqueSurfaceKHR surface{ vk::SurfaceKHR(_surface), _deleter };

    // Create device
    vkr::Device device{ instance, *surface };

    // Create swap chain
    vkr::SwapChain swapChain{ device, vk::Extent2D{ WIDTH, HEIGHT } };

    // Create storage image
    auto storageImage = swapChain.createStorageImage();

    // Create Mesh
    std::vector<vkr::Vertex> vertices{ { {1.0f, 1.0f, 0.0f} },
                                       { {-1.0f, 1.0f, 0.0f} },
                                       { {0.0f, -1.0f, 0.0f} } };
    std::vector<uint32_t> indices{ 0, 1, 2 };
    vkr::Mesh mesh(device, vertices, indices);

    // Build accel struct
    vkr::BottomLevelAccelerationStructure blas{ device, mesh };
    vkr::AccelerationStructureInstance asinstance{ 0, glm::mat4(1) };
    vkr::TopLevelAccelerationStructure tlas{ device, blas, asinstance };

    // Load shaders
    vkr::ShaderManager shaderManager{ device };
    shaderManager.addShader("samples/00_hello_triangle/raygen.rgen.spv", vk::ShaderStageFlagBits::eRaygenKHR, "main", vk::RayTracingShaderGroupTypeKHR::eGeneral);
    shaderManager.addShader("samples/00_hello_triangle/miss.rmiss.spv", vk::ShaderStageFlagBits::eMissKHR, "main", vk::RayTracingShaderGroupTypeKHR::eGeneral);
    shaderManager.addShader("samples/00_hello_triangle/closesthit.rchit.spv", vk::ShaderStageFlagBits::eClosestHitKHR, "main", vk::RayTracingShaderGroupTypeKHR::eTrianglesHitGroup);

    // Create desc sets
    vkr::DescriptorSets descSets{ device, 1 };
    descSets.addBindging(0, 0, vk::DescriptorType::eAccelerationStructureKHR, 1, vk::ShaderStageFlagBits::eRaygenKHR);
    descSets.addBindging(0, 1, vk::DescriptorType::eStorageImage, 1, vk::ShaderStageFlagBits::eRaygenKHR);
    descSets.initPipelineLayout();

    // Update desc sets
    descSets.allocate();
    descSets.addWriteInfo(0, 0, tlas.createWrite());
    descSets.addWriteInfo(0, 1, storageImage->createDescriptorInfo());
    descSets.update();

    auto pipeline = descSets.createRayTracingPipeline(shaderManager, 1);
    shaderManager.initShaderBindingTable(*pipeline, 1, 1, 1);
    swapChain.initDrawCommandBuffers(*pipeline, descSets, shaderManager, *storageImage);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        swapChain.draw();
    }
    device.waitIdle();
}
