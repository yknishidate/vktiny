#include "vktiny/vktiny.hpp"
#include <spdlog/spdlog.h>
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
    bufferDeviceAddressFeature.setPNext(&rayTracingPipelineFeaturesKHR);
    rayTracingPipelineFeaturesKHR.setPNext(&accelerationStructureFeaturesKHR);
    void* deviceCreatePNext = &bufferDeviceAddressFeature;

    // Init vulkan context
    context.initialize(VK_API_VERSION_1_2, true, width, height,
                       deviceExtensions, features, deviceCreatePNext);
}

vkt::Scene loadScene()
{
    vkt::Scene scene;
    scene.setMeshUsage(vkBU::eAccelerationStructureBuildInputReadOnlyKHR |
                       vkBU::eStorageBuffer | vkBU::eShaderDeviceAddress);
    scene.setMeshProperties(vkMP::eHostVisible | vkMP::eHostCoherent);
    scene.loadFile(context, "asset/Duck/Duck.gltf");

    // Output scene info
    for (auto&& mesh : scene.getMeshes()) {
        vkt::log::info("mesh");
        vkt::log::info("  vertices: {}", mesh.getVertices().size());
        vkt::log::info("  indices: {}", mesh.getIndices().size());
    }
    for (auto&& mat : scene.getMaterials()) {
        vkt::log::info("material");
        vkt::log::info("  baseColorFactor: {}", glm::to_string(mat.baseColorFactor));
        vkt::log::info("  baseColorTextureIndex: {}", mat.baseColorTextureIndex);
    }

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
                                   const std::vector<vkt::Mesh>& meshes)
{
    std::vector<MeshBuffers> meshData;
    for (const auto& mesh : meshes) {
        MeshBuffers data;
        data.vertices = mesh.getVertexBuffer().getDeviceAddress();
        data.indices = mesh.getIndexBuffer().getDeviceAddress();
        meshData.emplace_back(data);
    }

    vkt::Buffer sceneDesc;
    sceneDesc.initialize(context, sizeof(MeshBuffers) * static_cast<uint32_t>(meshes.size()),
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

int main()
{
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
    vkt::BottomLevelAccelStruct bottomLevelAS;
    vkt::TopLevelAccelStruct topLevelAS;
    bottomLevelAS.initialize(context, scene.getMeshes().front());
    topLevelAS.initialize(context, bottomLevelAS);

    // Create scene desc(binding = 3)
    vkt::Buffer sceneDesc = createBufferReferences(context, scene.getMeshes());

    // Add descriptor bindings
    descManager.addStorageImage(renderImage, 0);
    descManager.addTopLevelAccelStruct(topLevelAS, 1);
    descManager.addCombinedImageSamplers(scene.getTextures(), 2);
    descManager.addStorageBuffer(sceneDesc, 3);
    descManager.prepare();

    // Load shaders
    rtPipeline.addRaygenShader("shader/gltf_loading/spv/raygen.rgen.spv");
    rtPipeline.addMissShader("shader/gltf_loading/spv/miss.rmiss.spv");
    rtPipeline.addChitShader("shader/gltf_loading/spv/closesthit.rchit.spv");
    rtPipeline.prepare(descManager);

    // Build draw command buffers
    auto drawCommandBuffers = context.getSwapchain().allocateDrawComamndBuffers();
    for (int32_t i = 0; i < drawCommandBuffers.size(); ++i) {
        const auto& cmdBuf = drawCommandBuffers[i].get();
        const auto& swapchainImage = context.getSwapchain().getImages()[i];
        const auto& extent = context.getSwapchain().getExtent();

        cmdBuf.begin(vk::CommandBufferBeginInfo{});

        cmdBuf.bindPipeline(vk::PipelineBindPoint::eRayTracingKHR, rtPipeline.get());

        cmdBuf.bindDescriptorSets(vk::PipelineBindPoint::eRayTracingKHR, rtPipeline.getLayout(), 0,
                                  descManager.getDescSet(), nullptr);

        cmdBuf.traceRaysKHR(rtPipeline.getRaygenRegion(),
                            rtPipeline.getMissRegion(),
                            rtPipeline.getHitRegion(),
                            {}, width, height, 1);

        vkt::Image::transitionLayout(cmdBuf, renderImage.get(),
                                     vkIL::eUndefined, vkIL::eTransferSrcOptimal);
        vkt::Image::transitionLayout(cmdBuf, swapchainImage,
                                     vkIL::eUndefined, vkIL::eTransferDstOptimal);
        vkt::Image::copyImage(cmdBuf, renderImage.get(), swapchainImage, extent);
        vkt::Image::transitionLayout(cmdBuf, renderImage.get(),
                                     vkIL::eTransferSrcOptimal, vkIL::eGeneral);
        vkt::Image::transitionLayout(cmdBuf, swapchainImage,
                                     vkIL::eTransferDstOptimal, vkIL::ePresentSrcKHR);
        cmdBuf.end();
    }

    while (context.running()) {
        context.pollEvents();
        draw(drawCommandBuffers);
    }
    context.getDevice().waitIdle();
}
