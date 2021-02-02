
#include "../../vkray.hpp"

#include <glm/gtc/constants.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

using vkss = vk::ShaderStageFlagBits;
using vkdt = vk::DescriptorType;
using vksgt = vk::RayTracingShaderGroupTypeKHR;

struct UniformData
{
    glm::mat4 model;
    glm::mat4 invView;
    glm::mat4 invProj;
};

struct Camera
{
    float phi{ 0 };
    float theta{ 0 };

    float fov{ 45 };
    float aspect{ 4.0f / 3.0f };
    float znear{ 0.001 };
    float zfar{ 1000 };

    glm::vec4 pos{ 0, 0, 3, 1 };
    glm::vec3 target{ 0, -0.25, 0 };
    glm::vec3 up{ 0, 1, 0 };
    glm::mat4 invView{ 1 };
    glm::mat4 invProj{ 1 };

    Camera()
    {
        invView = glm::inverse(glm::lookAt(glm::vec3(pos), target, up));
        invProj = glm::inverse(glm::perspective(glm::radians(fov), aspect, znear, zfar));
    }

    void update(float dx, float dy)
    {
        phi -= dx;
        theta = std::clamp(theta + dy, -89.0f, 89.0f);

        glm::mat4 rotX = glm::rotate(glm::radians(theta), glm::vec3(1, 0, 0));
        glm::mat4 rotY = glm::rotate(glm::radians(phi), glm::vec3(0, 1, 0));

        invView = glm::inverse(glm::lookAt(glm::vec3(rotY * rotX * pos), target, up));
        invProj = glm::inverse(glm::perspective(glm::radians(fov), aspect, znear, zfar));
    }

};

class Application
{
public:

    void onCursorPosition(double xpos, double ypos)
    {
        if (nowPressed) {
            camera.update(xpos - lastCursorPos.x, ypos - lastCursorPos.y);
            lastCursorPos = glm::vec2(xpos, ypos);
        }
    }

    void onMouseButton(int button, int action, int mods)
    {
        // left(0), right(1)
        if (button == 0) {
            // press(1), release(0)
            nowPressed = bool(action);
            lastCursorPos = window->getCursorPos();
        }
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
        uniformData.invView = camera.invView;
        uniformData.invProj = camera.invProj;
        ubo->copy(&uniformData);
    }

    void run()
    {
        window = std::make_unique<vkr::Window>("vkray", 800, 600);
        instance = std::make_unique<vkr::Instance>(*window, true);
        device = std::make_unique<vkr::Device>(*instance);
        swapChain = std::make_unique<vkr::SwapChain>(*device, *window);

        window->onCursorPosition = [this](const double xpos, const double ypos) { onCursorPosition(xpos, ypos); };
        window->onMouseButton = [this](const int button, const int action, const int mods) { onMouseButton(button, action, mods); };

        // Create storage image
        storageImage = swapChain->createStorageImage();

        // Create BLAS
        vkr::Mesh mesh;
        glm::vec3 normal(1);
        glm::vec2 uv(1);
        std::vector<vkr::Vertex> vertices{
            vkr::Vertex{glm::vec3(0,  0, 0), normal, uv, glm::vec4(1, 1, 1, 1)},
            vkr::Vertex{glm::vec3(1,  0, 0), normal, uv, glm::vec4(1, 0, 0, 1)},
            vkr::Vertex{glm::vec3(0, -1, 0), normal, uv, glm::vec4(0, 1, 0, 1)},
            vkr::Vertex{glm::vec3(0,  0, 1), normal, uv, glm::vec4(0, 0, 1, 1)}
        };
        std::vector<uint32_t> indices{
            0, 1, 2,
            0, 2, 3,
            0, 3, 1
        };
        mesh.create(*device, vertices, indices);

        blas = std::make_unique<vkr::BottomLevelAccelerationStructure>(*device, mesh);

        // Create TLAS
        glm::mat4 worldMatrix{ 1 };
        vkr::AccelerationStructureInstance instance{ 0, worldMatrix };

        tlas = std::make_unique<vkr::TopLevelAccelerationStructure>(*device, *blas, instance);

        createUniformBuffer();

        // Load shaders
        shaderManager = std::make_unique<vkr::ShaderManager>(*device);
        shaderManager->addShader("samples/03_ubo_camera/raygen.rgen.spv", vkss::eRaygenKHR, "main", vksgt::eGeneral);
        shaderManager->addShader("samples/03_ubo_camera/miss.rmiss.spv", vkss::eMissKHR, "main", vksgt::eGeneral);
        shaderManager->addShader("samples/03_ubo_camera/closesthit.rchit.spv", vkss::eClosestHitKHR, "main", vksgt::eTrianglesHitGroup);

        // Create Desc Sets
        descSets = std::make_unique<vkr::DescriptorSets>(*device, 1);
        descSets->addBindging(0, 0, vkdt::eAccelerationStructureKHR, 1, vkss::eRaygenKHR); // TLAS
        descSets->addBindging(0, 1, vkdt::eStorageImage, 1, vkss::eRaygenKHR);             // Image
        descSets->addBindging(0, 5, vkdt::eUniformBuffer, 1, vkss::eRaygenKHR);            // UBO

        descSets->initPipelineLayout();

        descSets->allocate();
        descSets->addWriteInfo(0, 0, tlas->createWrite());
        descSets->addWriteInfo(0, 1, storageImage->createDescriptorInfo());
        descSets->addWriteInfo(0, 5, ubo->createDescriptorInfo());
        descSets->update();

        // Create Ray Tracing Pipeline
        pipeline = device->createRayTracingPipeline(*descSets, *shaderManager, 1);

        // Init Shader Binding Table
        shaderManager->initShaderBindingTable(*pipeline, 0, 1, 2);

        // Init Draw Command Buffers
        swapChain->initDrawCommandBuffers(*pipeline, *descSets, *shaderManager, *storageImage);

        // Main loop
        while (!window->shouldClose()) {
            ++frame;
            window->pollEvents();
            swapChain->draw();

            updateUniformBuffer();
        }

        device->waitIdle();

    }

private:

    uint64_t frame{ 0 };

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

    UniformData uniformData;
    std::unique_ptr<vkr::Buffer> ubo;

    // for mouse input
    glm::vec2 lastCursorPos;
    bool nowPressed = false;

    Camera camera;

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
