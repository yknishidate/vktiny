
#define QUICK_VKRAY_IMPLEMENTATION
#include "../../vkray.hpp"

#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

using vkss = vk::ShaderStageFlagBits;
using vkdt = vk::DescriptorType;
using vksgt = vk::RayTracingShaderGroupTypeKHR;

constexpr int WIDTH = 800;
constexpr int HEIGHT = 600;

struct UniformData
{
    glm::mat4 invView;
    glm::mat4 invProj;
};

struct Camera
{
    glm::vec4 position;
    glm::vec3 target;

    glm::mat4 view;
    glm::mat4 proj;

    float fov = 45;
    float aspect;

    float phi = 0;
    float theta = 0;

    Camera()
    {
        position = glm::vec4(0, 0, 6, 1);
        target = glm::vec3(0);
        aspect = float(WIDTH) / HEIGHT;
        update();
    }

    void update()
    {
        glm::mat4 rotX = glm::rotate(glm::radians(theta), glm::vec3(1, 0, 0));
        glm::mat4 rotY = glm::rotate(glm::radians(phi), glm::vec3(0, 1, 0));

        view = glm::lookAt(glm::vec3(rotY * rotX * position), target, glm::vec3(0, 1, 0));
        proj = glm::perspective(glm::radians(fov), aspect, 0.001f, 10000.0f);
    }

    void processMouseMotion(float dx, float dy)
    {
        phi = glm::mod(phi - dx, 360.0f);
        theta = std::min(std::max(theta + dy, -89.9f), 89.9f);
        update();
    }

    void processMouseWheel(float value)
    {
        position.z = position.z - value;
        update();
    }
};

namespace
{
    void keyCallback(GLFWwindow* window, const int key, const int scancode, const int action, const int mods);
    void cursorPositionCallback(GLFWwindow* window, const double xpos, const double ypos);
    void mouseButtonCallback(GLFWwindow* window, const int button, const int action, const int mods);
    void scrollCallback(GLFWwindow* window, const double xoffset, const double yoffset);
}

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

    Camera camera;

    glm::vec2 lastCursorPos;

    bool nowPressed = false;

private:

    GLFWwindow* window;

    std::unique_ptr<vkr::Instance> instance;

    vk::UniqueSurfaceKHR surface;

    std::unique_ptr<vkr::Device> device;

    std::unique_ptr<vkr::SwapChain> swapChain;

    std::unique_ptr<vkr::Image> storageImage;

    std::unique_ptr<vkr::BottomLevelAccelerationStructure> blas;
    vkr::AccelerationStructureInstance asInstance;
    std::unique_ptr<vkr::TopLevelAccelerationStructure> tlas;

    std::unique_ptr<vkr::ShaderManager> shaderManager;

    std::unique_ptr<vkr::DescriptorSets> descSets;

    vk::UniquePipeline pipeline;

    vkr::Model model;

    UniformData uniformData;
    std::unique_ptr<vkr::Buffer> ubo;

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

        createUniformBuffer();

        createDescSets();

        pipeline = descSets->createRayTracingPipeline(*shaderManager, 1);
        shaderManager->initShaderBindingTable(*pipeline, 1, 1, 1);
        swapChain->initDrawCommandBuffers(*pipeline, *descSets, *shaderManager, *storageImage);

        glfwSetWindowUserPointer(window, this);
        glfwSetKeyCallback(window, keyCallback);
        glfwSetCursorPosCallback(window, cursorPositionCallback);
        glfwSetMouseButtonCallback(window, mouseButtonCallback);
        glfwSetScrollCallback(window, scrollCallback);
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

        asInstance = vkr::AccelerationStructureInstance{ 0, glm::mat4(1) };

        tlas = std::make_unique<vkr::TopLevelAccelerationStructure>(*device, *blas, asInstance);
    }

    void loadShaders()
    {
        shaderManager = std::make_unique<vkr::ShaderManager>(*device);
        shaderManager->addShader("samples/02_mouse_input_and_camera/raygen.rgen.spv", vkss::eRaygenKHR, "main", vksgt::eGeneral);
        shaderManager->addShader("samples/02_mouse_input_and_camera/miss.rmiss.spv", vkss::eMissKHR, "main", vksgt::eGeneral);
        shaderManager->addShader("samples/02_mouse_input_and_camera/closesthit.rchit.spv", vkss::eClosestHitKHR, "main", vksgt::eTrianglesHitGroup);
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
        descSets->addBindging(0, 5, vkdt::eUniformBuffer, 1, vkss::eRaygenKHR);            // UBO
        descSets->initPipelineLayout();

        descSets->allocate();
        descSets->addWriteInfo(0, 0, tlas->createWrite());
        descSets->addWriteInfo(0, 1, storageImage->createDescriptorInfo());
        descSets->addWriteInfo(0, 2, mesh.vertexBuffer->createDescriptorInfo());
        descSets->addWriteInfo(0, 3, mesh.indexBuffer->createDescriptorInfo());
        descSets->addWriteInfo(0, 4, texture.createDescriptorInfo());
        descSets->addWriteInfo(0, 5, ubo->createDescriptorInfo());
        descSets->update();
    }

    void createUniformBuffer()
    {
        vk::DeviceSize size = sizeof(UniformData);
        vk::BufferUsageFlags usage{ vk::BufferUsageFlagBits::eUniformBuffer };
        vk::MemoryPropertyFlags prop{ vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };
        ubo = std::make_unique<vkr::Buffer>(*device, size, usage, prop, &uniformData);

        updateUniformBuffer();
    }

    void updateUniformBuffer()
    {
        uniformData.invView = glm::inverse(camera.view);
        uniformData.invProj = glm::inverse(camera.proj);
        ubo->copy(&uniformData);
    }

    void updateTLAS()
    {
        asInstance.transformMatrix = glm::rotate(asInstance.transformMatrix, glm::radians(1.0f), glm::vec3(0, 1, 0));
        asInstance.transformMatrix = glm::rotate(asInstance.transformMatrix, glm::radians(3.0f), glm::vec3(1, 0, 0));
        tlas->update(*device, asInstance);
    }

    void mainLoop()
    {
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
            swapChain->draw();
            updateUniformBuffer();
            updateTLAS();
        }
        device->waitIdle();
    }

};

namespace
{
    void keyCallback(GLFWwindow* window, const int key, const int scancode, const int action, const int mods)
    {
        auto* const this_ = static_cast<Application*>(glfwGetWindowUserPointer(window));
    }

    void cursorPositionCallback(GLFWwindow* window, const double xpos, const double ypos)
    {
        auto* const this_ = static_cast<Application*>(glfwGetWindowUserPointer(window));
        if (this_->nowPressed) {
            this_->camera.processMouseMotion(xpos - this_->lastCursorPos.x, ypos - this_->lastCursorPos.y);
            this_->lastCursorPos = glm::vec2(xpos, ypos);
        }
    }

    void mouseButtonCallback(GLFWwindow* window, const int button, const int action, const int mods)
    {
        auto* const this_ = static_cast<Application*>(glfwGetWindowUserPointer(window));
        if (button == 0) {
            this_->nowPressed = bool(action);
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            this_->lastCursorPos = glm::vec2(xpos, ypos);
        }
    }

    void scrollCallback(GLFWwindow* window, const double xoffset, const double yoffset)
    {
        auto* const this_ = static_cast<Application*>(glfwGetWindowUserPointer(window));
        this_->camera.processMouseWheel(float(yoffset));
    }
}

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
