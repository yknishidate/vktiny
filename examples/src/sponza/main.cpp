#include "vktiny/vktiny.hpp"
#include <glm/gtx/string_cast.hpp>
#include <utility>

using vkIL = vk::ImageLayout;
using vkIU = vk::ImageUsageFlagBits;
using vkBU = vk::BufferUsageFlagBits;
using vkMP = vk::MemoryPropertyFlagBits;

vkt::Context context;
const int width = 1280;
const int height = 720;

struct MeshBuffers
{
    vk::DeviceAddress vertices;
    vk::DeviceAddress indices;
    int baseColorTexture;
};

struct UniformData
{
    glm::mat4 invView;
    glm::mat4 invProj;
    glm::vec3 lightDirection;
};

void initContext()
{
    // Add device extensions
    std::vector<const char*> deviceExtensions;
    deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    deviceExtensions.push_back(VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME);
    deviceExtensions.push_back(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
    deviceExtensions.push_back(VK_KHR_MAINTENANCE3_EXTENSION_NAME);
    deviceExtensions.push_back(VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME);
    deviceExtensions.push_back(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
    deviceExtensions.push_back(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    deviceExtensions.push_back(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
    deviceExtensions.push_back(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);

    // Add physical device features
    vk::PhysicalDeviceFeatures features;
    features.shaderInt64 = true;

    vk::PhysicalDeviceBufferDeviceAddressFeatures bufferDeviceAddressFeature{ true };
    vk::PhysicalDeviceRayTracingPipelineFeaturesKHR rayTracingPipelineFeaturesKHR{ true };
    vk::PhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeaturesKHR{ true };
    vk::PhysicalDeviceDescriptorIndexingFeatures descriptorIndexingFeatures;
    descriptorIndexingFeatures.runtimeDescriptorArray = true;

    bufferDeviceAddressFeature.setPNext(&rayTracingPipelineFeaturesKHR);
    rayTracingPipelineFeaturesKHR.setPNext(&accelerationStructureFeaturesKHR);
    accelerationStructureFeaturesKHR.setPNext(&descriptorIndexingFeatures);
    void* deviceCreatePNext = &bufferDeviceAddressFeature;

    // Init vulkan context
    context.initialize(VK_API_VERSION_1_2, true, width, height,
                       deviceExtensions, features, deviceCreatePNext);

    context.setWindowIcon("asset/vulkan.png");
}

vkt::Scene loadScene()
{
    vkt::Scene scene;
    scene.setMeshUsage(vkBU::eAccelerationStructureBuildInputReadOnlyKHR |
                       vkBU::eStorageBuffer | vkBU::eShaderDeviceAddress);
    scene.setMeshProperties(vkMP::eHostVisible | vkMP::eHostCoherent);
    scene.loadFile(context, "asset/sponza.gltf");

    // Output scene info
    vkt::log::info("mesh    : {}", scene.getMeshes().size());
    vkt::log::info("material: {}", scene.getMaterials().size());
    vkt::log::info("texture : {}", scene.getTextures().size());

    return scene;
}

vkt::Image createRenderImage()
{
    vkt::Image renderImage;
    renderImage.initialize(context,
                           context.getSwapchain().getExtent(),
                           context.getSwapchain().getFormat(),
                           vkIU::eStorage | vkIU::eTransferSrc | vkIU::eTransferDst);
    renderImage.createImageView();
    renderImage.transitionLayout(vk::ImageLayout::eGeneral);
    return renderImage;
}

vkt::Buffer createBufferReferences(const vkt::Context& context,
                                   const vkt::Scene& scene)
{
    auto&& meshes = scene.getMeshes();
    std::vector<MeshBuffers> meshData;
    for (const auto& mesh : meshes) {
        MeshBuffers data;
        data.vertices = mesh.getVertexBuffer().getDeviceAddress();
        data.indices = mesh.getIndexBuffer().getDeviceAddress();
        auto&& mat = scene.getMaterials()[mesh.getMaterialIndex()];
        data.baseColorTexture = mat.baseColorTextureIndex;
        meshData.emplace_back(data);
    }

    vkt::Buffer sceneDesc;
    sceneDesc.initialize(context, sizeof(MeshBuffers) * meshes.size(),
                         vkBU::eStorageBuffer | vkBU::eShaderDeviceAddress,
                         vkMP::eHostVisible | vkMP::eHostCoherent,
                         meshData.data());
    return sceneDesc;
}

void draw(std::vector<vk::UniqueCommandBuffer>& drawCommandBuffers)
{
    // Begin
    vkt::FrameInfo frameInfo = context.getSwapchain().beginFrame();
    const auto& cmdBuf = *drawCommandBuffers[frameInfo.imageIndex];

    // Render
    vk::PipelineStageFlags waitStage{ vk::PipelineStageFlagBits::eRayTracingShaderKHR };
    vk::SubmitInfo submitInfo;
    submitInfo.setWaitSemaphores(frameInfo.imageAvailableSemaphore);
    submitInfo.setWaitDstStageMask(waitStage);
    submitInfo.setCommandBuffers(cmdBuf);
    submitInfo.setSignalSemaphores(frameInfo.renderFinishedSemaphore);
    context.getDevice().getGraphicsQueue().submit(submitInfo, frameInfo.inFlightFence);

    // End
    context.getSwapchain().endFrame(frameInfo.imageIndex, cmdBuf);
}

void copyImage(vk::CommandBuffer cmdBuf, vk::Image src, vk::Image dst)
{
    vkt::Image::transitionLayout(cmdBuf, src,
                                 vkIL::eUndefined, vkIL::eTransferSrcOptimal);
    vkt::Image::transitionLayout(cmdBuf, dst,
                                 vkIL::eUndefined, vkIL::eTransferDstOptimal);
    vkt::Image::copyImage(cmdBuf, src, dst, context.getSwapchain().getExtent());
    vkt::Image::transitionLayout(cmdBuf, src,
                                 vkIL::eTransferSrcOptimal, vkIL::eGeneral);
    vkt::Image::transitionLayout(cmdBuf, dst,
                                 vkIL::eTransferDstOptimal, vkIL::ePresentSrcKHR);
}

void updateUniformBuffer(vkt::OrbitalCamera& camera, UniformData& uniformData, vkt::Buffer& uniformBuffer)
{
    if (context.getInput().mousePressed[0]) {
        camera.processCursorMotion(context.getInput().xoffset, context.getInput().yoffset);
    }
    camera.processMouseWheel(context.getInput().scroll);

    camera.update();
    uniformData.invView = glm::inverse(camera.view);
    uniformData.invProj = glm::inverse(camera.proj);
    uniformData.lightDirection = glm::normalize(glm::vec3(0, 1, 0.5));
    uniformBuffer.copy(&uniformData);
}

int main()
{
    try {
        initContext();

        vkt::DescriptorManager descManager;
        vkt::RayTracingPipeline rtPipeline;
        descManager.initialize(context);
        rtPipeline.initialize(context);

        // Load scene
        vkt::Scene scene = loadScene();

        // Create render image(binding = 0)
        vkt::Image renderImage = createRenderImage();

        // Create accel structs(binding = 1)
        std::vector<vkt::BottomLevelAccelStruct> bottomLevelASs;
        bottomLevelASs.reserve(scene.getMeshes().size());
        for (auto& mesh : scene.getMeshes()) {
            bottomLevelASs.push_back(vkt::BottomLevelAccelStruct{});
            bottomLevelASs.back().initialize(context, mesh);
        }
        glm::mat4 transform = vkt::flipY(glm::translate(glm::mat4(1.0), { 0, -2, 0 }));
        vkt::TopLevelAccelStruct topLevelAS;
        topLevelAS.initialize(context, bottomLevelASs, transform);

        // Create scene desc(binding = 3)
        vkt::Buffer sceneDesc = createBufferReferences(context, scene);

        // Create uniform data(binding = 4)
        vkt::OrbitalCamera camera(width, height, 8);
        camera.theta = 9.0;
        camera.phi = 100.0;
        UniformData uniformData;
        uniformData.invView = glm::inverse(camera.view);
        uniformData.invProj = glm::inverse(camera.proj);
        vkt::Buffer uniformBuffer;
        uniformBuffer.initialize(context, sizeof(UniformData), vkBU::eUniformBuffer,
                                 vkMP::eHostVisible | vkMP::eHostCoherent, &uniformData);

        // Add descriptor bindings
        descManager.addStorageImage(renderImage, 0);
        descManager.addTopLevelAccelStruct(topLevelAS, 1);
        descManager.addCombinedImageSamplers(scene.getTextures(), 2);
        descManager.addStorageBuffer(sceneDesc, 3);
        descManager.addUniformBuffer(uniformBuffer, 4);
        descManager.prepare();

        // Load shaders
        rtPipeline.addRaygenShader("shader/sponza/spv/raygen.rgen.spv");
        rtPipeline.addMissShader("shader/sponza/spv/miss.rmiss.spv");
        rtPipeline.addChitShader("shader/sponza/spv/closesthit.rchit.spv");
        rtPipeline.prepare(descManager);

        // Build draw command buffers
        auto drawCommandBuffers = context.getSwapchain().allocateDrawComamndBuffers();
        for (int32_t i = 0; i < drawCommandBuffers.size(); ++i) {
            const auto& cmdBuf = drawCommandBuffers[i].get();
            cmdBuf.begin(vk::CommandBufferBeginInfo{});
            cmdBuf.bindPipeline(vk::PipelineBindPoint::eRayTracingKHR, rtPipeline.get());
            cmdBuf.bindDescriptorSets(vk::PipelineBindPoint::eRayTracingKHR, rtPipeline.getLayout(), 0,
                                      descManager.getDescSet(), nullptr);
            cmdBuf.traceRaysKHR(rtPipeline.getRaygenRegion(),
                                rtPipeline.getMissRegion(),
                                rtPipeline.getHitRegion(),
                                {}, width, height, 1);
            copyImage(cmdBuf, renderImage.get(), context.getSwapchain().getImages()[i]);
            cmdBuf.end();
        }

        uint64_t frame = 0;
        while (context.running()) {
            context.pollEvents();
            draw(drawCommandBuffers);
            updateUniformBuffer(camera, uniformData, uniformBuffer);
            frame++;
        }
        context.getDevice().waitIdle();
    } catch (const std::exception& e) {
        vkt::log::error(e.what());
    }
}
