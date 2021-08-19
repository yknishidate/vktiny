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
};

struct UniformData
{
    glm::mat4 invView;
    glm::mat4 invProj;
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

    // Create uniform data(binding = 4)
    vkt::OrbitalCamera camera(width, height, 400);
    camera.theta = -25.0;
    UniformData uniformData;
    uniformData.invView = glm::inverse(camera.view);
    uniformData.invProj = glm::inverse(camera.proj);
    vkt::Buffer uniformBuffer;
    uniformBuffer.initialize(context, sizeof(UniformData), vkBU::eUniformBuffer,
                             vkMP::eHostVisible | vkMP::eHostCoherent, &uniformData);

    // Set input callbacks
    bool mousePressed = false;
    double xmove, ymove;

    vkt::Input input;
    input.initialize(context);
    input.setOnCursorPosition(
        [&](const double xpos, const double ypos) {
            static double x = xpos;
            static double y = ypos;
            xmove = xpos - x;
            ymove = ypos - y;
            x = xpos;
            y = ypos;
            if (mousePressed) {
                vkt::log::info("cursor move: {} {}", xmove, ymove);
            }
        });
    input.setOnMouseButton(
        [&](const int button, const int action, const int mods) {
            if (action == vkt::InputState::Press) {
                vkt::log::info("pressed: {}", button);
                mousePressed = true;
            } else {
                vkt::log::info("released: {}", button);
                mousePressed = false;
            }
        });

    // Add descriptor bindings
    descManager.addStorageImage(renderImage, 0);
    descManager.addTopLevelAccelStruct(topLevelAS, 1);
    descManager.addCombinedImageSamplers(scene.getTextures(), 2);
    descManager.addStorageBuffer(sceneDesc, 3);
    descManager.addUniformBuffer(uniformBuffer, 4);
    descManager.prepare();

    // Load shaders
    rtPipeline.addRaygenShader("shader/camera/spv/raygen.rgen.spv");
    rtPipeline.addMissShader("shader/camera/spv/miss.rmiss.spv");
    rtPipeline.addChitShader("shader/camera/spv/closesthit.rchit.spv");
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

        camera.phi += 1.0;
        camera.update();
        uniformData.invView = glm::inverse(vkt::flipY(camera.view));
        uniformData.invProj = glm::inverse(vkt::flipY(camera.proj));
        uniformBuffer.copy(&uniformData);
        frame++;
    }
    context.getDevice().waitIdle();
}
