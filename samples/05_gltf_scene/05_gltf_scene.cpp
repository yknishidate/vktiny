
#define QUICK_VKRAY_IMPLEMENTATION
#include "../../vkray.hpp"

#include <glm/gtc/constants.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

using vkss = vk::ShaderStageFlagBits;
using vkdt = vk::DescriptorType;
using vksgt = vk::RayTracingShaderGroupTypeKHR;

constexpr float WIDTH = 1280;
constexpr float HEIGHT = 720;

struct UniformData
{
    glm::mat4 model;
    glm::mat4 invView;
    glm::mat4 invProj;
};

struct Camera
{
    const float scale{ 10 };

    float phi{ 0 };
    float theta{ 0 };

    float fov{ 45 };
    float aspect{ WIDTH / HEIGHT };
    float znear{ 0.001f * scale };
    float zfar{ 1000.0f * scale };

    glm::vec4 pos{ 0, 0, 2 * scale, 1 };
    glm::vec3 target{ 0, -2, 0 };
    glm::vec3 up{ 0, 1, 0 };
    glm::mat4 invView{ 1 };
    glm::mat4 invProj{ 1 };
    glm::mat4 rotX{ 1.0f };
    glm::mat4 rotY{ 1.0f };

    Camera()
    {
        invView = glm::inverse(glm::lookAt(glm::vec3(pos), target, up));
        invProj = glm::inverse(glm::perspective(glm::radians(fov), aspect, znear, zfar));
    }

    void update(float dx, float dy)
    {
        phi -= dx;
        theta = std::clamp(theta + dy, -89.0f, 89.0f);

        rotX = glm::rotate(glm::radians(theta), glm::vec3(1, 0, 0));
        rotY = glm::rotate(glm::radians(phi), glm::vec3(0, 1, 0));

        invView = glm::inverse(glm::lookAt(glm::vec3(rotY * rotX * pos), target, up));
        invProj = glm::inverse(glm::perspective(glm::radians(fov), aspect, znear, zfar));
    }

    void update(float yoffset)
    {
        pos.z -= yoffset / 4.0 * scale;
        invView = glm::inverse(glm::lookAt(glm::vec3(rotY * rotX * pos), target, up));
        invProj = glm::inverse(glm::perspective(glm::radians(fov), aspect, znear, zfar));
    }
};

struct InstanceDataOnDevice
{
    glm::mat4 worldMatrix{ 1.0f };
    int meshIndex{ -1 };
    int baseColorTextureIndex{ -1 };
    int normalTextureIndex{ -1 };
    int occlusionTextureIndex{ -1 };
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

    void onScroll(double xoffset, double yoffset)
    {
        camera.update(float(yoffset));
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

    void createInstanceDataBuffer(vkr::Model& model)
    {
        const std::vector<vkr::Node>& nodes = model.getNodes();
        const std::vector<vkr::Mesh>& meshes = model.getMeshes();
        const std::vector<vkr::Material>& materials = model.getMaterials();
        const std::vector<vkr::Texture>& textures = model.getTextures();

        vk::DeviceSize size = sizeof(InstanceDataOnDevice);
        vk::BufferUsageFlags usage{ vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eTransferDst };
        vk::MemoryPropertyFlags prop{ vk::MemoryPropertyFlagBits::eDeviceLocal };

        for (const auto& node : nodes) {
            const auto& mesh = meshes[node.meshIndex];
            const auto& material = materials[mesh.materialIndex];

            InstanceDataOnDevice data;
            data.meshIndex = node.meshIndex;
            data.worldMatrix = node.worldMatrix;
            data.baseColorTextureIndex = material.baseColorTextureIndex;
            data.normalTextureIndex = material.normalTextureIndex;
            data.occlusionTextureIndex = material.occlusionTextureIndex;

            instanceDataBuffers.push_back(
                std::make_unique<vkr::Buffer>(*device, size, usage, prop, &data));
        }

    }

    void run()
    {
        window = std::make_unique<vkr::Window>("vkray", WIDTH, HEIGHT);
        instance = std::make_unique<vkr::Instance>(*window, true);
        device = std::make_unique<vkr::Device>(*instance);
        swapChain = std::make_unique<vkr::SwapChain>(*device, *window);

        window->onCursorPosition = [this](const double xpos, const double ypos) {
            onCursorPosition(xpos, ypos);
        };
        window->onMouseButton = [this](const int button, const int action, const int mods) {
            onMouseButton(button, action, mods);
        };
        window->onScroll = [this](const double xoffset, const double yoffset) {
            onScroll(xoffset, yoffset);
        };

        // Create storage image
        storageImage = swapChain->createStorageImage();

        vkr::Model model;
        model.loadFromFile(*device, "samples/assets/Sponza/Sponza.gltf");

        const std::vector<vkr::Node>& nodes = model.getNodes();
        const std::vector<vkr::Mesh>& meshes = model.getMeshes();
        const std::vector<vkr::Material>& materials = model.getMaterials();
        const std::vector<vkr::Texture>& textures = model.getTextures();

        std::cout << "Nodes     : " << nodes.size() << std::endl;
        std::cout << "Meshes    : " << meshes.size() << std::endl;
        std::cout << "Materials : " << materials.size() << std::endl;
        std::cout << "Textures  : " << textures.size() << std::endl;

        // Build BLASs
        for (const auto& mesh : meshes) {
            blasArray.push_back(std::make_unique<vkr::BottomLevelAccelerationStructure>(*device, mesh));
        }

        // Create AS instances
        std::vector<vkr::AccelerationStructureInstance> instances;
        for (const auto& node : nodes) {
            instances.push_back({ static_cast<uint32_t>(node.meshIndex), node.worldMatrix });
        }

        // Create TLAS
        tlas = std::make_unique<vkr::TopLevelAccelerationStructure>(*device, blasArray, instances);

        createInstanceDataBuffer(model);
        createUniformBuffer();

        // Load shaders
        shaderManager = std::make_unique<vkr::ShaderManager>(*device);
        shaderManager->addShader("samples/05_gltf_scene/raygen.rgen.spv", vkss::eRaygenKHR, "main", vksgt::eGeneral);
        shaderManager->addShader("samples/05_gltf_scene/miss.rmiss.spv", vkss::eMissKHR, "main", vksgt::eGeneral);
        shaderManager->addShader("samples/05_gltf_scene/closesthit.rchit.spv", vkss::eClosestHitKHR, "main", vksgt::eTrianglesHitGroup);

        // Create Desc Sets
        descSets = std::make_unique<vkr::DescriptorSets>(*device, 1);
        descSets->addBindging(0, 0, vkdt::eAccelerationStructureKHR, 1, vkss::eRaygenKHR);               // TLAS
        descSets->addBindging(0, 1, vkdt::eStorageImage, 1, vkss::eRaygenKHR);                           // Image
        descSets->addBindging(0, 2, vkdt::eUniformBuffer, 1, vkss::eRaygenKHR);                          // UBO
        descSets->addBindging(0, 3, vkdt::eStorageBuffer, meshes.size(), vkss::eClosestHitKHR);          // Vertex
        descSets->addBindging(0, 4, vkdt::eStorageBuffer, meshes.size(), vkss::eClosestHitKHR);          // Index
        descSets->addBindging(0, 5, vkdt::eCombinedImageSampler, textures.size(), vkss::eClosestHitKHR); // Texture
        descSets->addBindging(0, 6, vkdt::eUniformBuffer, nodes.size(), vkss::eClosestHitKHR);           // Instance data

        descSets->initPipelineLayout();

        // Create DescInfo array
        std::vector<vk::DescriptorBufferInfo> vertexBufferInfo;
        std::vector<vk::DescriptorBufferInfo> indexBufferInfo;
        for (const auto& mesh : meshes) {
            vertexBufferInfo.push_back(mesh.vertexBuffer->createDescriptorInfo());
            indexBufferInfo.push_back(mesh.indexBuffer->createDescriptorInfo());
        }

        std::vector<vk::DescriptorImageInfo> textureInfo;
        for (const auto& tex : textures) {
            textureInfo.push_back(tex.createDescriptorInfo());
        }

        std::vector<vk::DescriptorBufferInfo> instanceDataBufferInfo;
        for (const auto& bufffer : instanceDataBuffers) {
            instanceDataBufferInfo.push_back(bufffer->createDescriptorInfo());
        }

        descSets->allocate();
        descSets->addWriteInfo(0, 0, tlas->createWrite());
        descSets->addWriteInfo(0, 1, storageImage->createDescriptorInfo());
        descSets->addWriteInfo(0, 2, ubo->createDescriptorInfo());
        descSets->addWriteInfo(0, 3, vertexBufferInfo);
        descSets->addWriteInfo(0, 4, indexBufferInfo);
        descSets->addWriteInfo(0, 5, textureInfo);
        descSets->addWriteInfo(0, 6, instanceDataBufferInfo);
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

    std::vector<std::unique_ptr<vkr::BottomLevelAccelerationStructure>> blasArray;
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

    std::vector<std::unique_ptr<vkr::Buffer>> instanceDataBuffers;
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
