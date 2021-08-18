#include "vktiny/vktiny.hpp"

using vkIL = vk::ImageLayout;
using vkIU = vk::ImageUsageFlagBits;
using vkBU = vk::BufferUsageFlagBits;
using vkMP = vk::MemoryPropertyFlagBits;

int main()
{
    int width = 1280;
    int height = 720;

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
    vk::PhysicalDeviceBufferDeviceAddressFeatures bufferDeviceAddressFeature{ true };
    vk::PhysicalDeviceRayTracingPipelineFeaturesKHR rayTracingPipelineFeaturesKHR{ true };
    vk::PhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeaturesKHR{ true };
    bufferDeviceAddressFeature.setPNext(&rayTracingPipelineFeaturesKHR);
    rayTracingPipelineFeaturesKHR.setPNext(&accelerationStructureFeaturesKHR);
    void* deviceCreatePNext = &bufferDeviceAddressFeature;

    // Init vulkan context
    vkt::Context context;
    context.initialize(VK_API_VERSION_1_2, true, width, height,
                       deviceExtensions, deviceCreatePNext);

    vkt::DescriptorManager descManager;
    descManager.initialize(context);

    vkt::RayTracingPipeline rtPipeline;
    rtPipeline.initialize(context);

    // Create render image
    vkt::Image renderImage;
    renderImage.initialize(context,
                           context.getSwapchain().getExtent(),
                           context.getSwapchain().getFormat(),
                           vkIU::eStorage | vkIU::eTransferSrc | vkIU::eTransferDst);
    renderImage.createImageView();
    renderImage.transitionLayout(vk::ImageLayout::eGeneral);

    // Create vertices and indices
    std::vector<vkt::Vertex> vertices{ { { 0.0, -0.3, 0.0} },
                                       { { 0.3,  0.3, 0.0} },
                                       { {-0.3,  0.3, 0.0} } };
    std::vector<vkt::Index> indices{ 0, 1, 2 };

    // Create vertex and index buffer
    vk::BufferUsageFlags usage{ vkBU::eAccelerationStructureBuildInputReadOnlyKHR |
                                vkBU::eStorageBuffer | vkBU::eShaderDeviceAddress };
    vk::MemoryPropertyFlags props{ vkMP::eHostVisible | vkMP::eHostCoherent };
    vkt::Mesh mesh;
    mesh.initialize(context, vertices, indices, usage, props);

    // Create accel structs
    vkt::BottomLevelAccelStruct bottomLevelAS;
    vkt::TopLevelAccelStruct topLevelAS;
    bottomLevelAS.initialize(context, mesh);
    topLevelAS.initialize(context, bottomLevelAS);

    // Add descriptor bindings
    descManager.addStorageImage(renderImage, 0);
    descManager.addTopLevelAccelStruct(topLevelAS, 1);
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
    context.getDevice().waitIdle();
}
