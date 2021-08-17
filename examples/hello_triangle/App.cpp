#include "App.hpp"
#include <iostream>
#include <set>

using vkIL = vk::ImageLayout;
using vkIU = vk::ImageUsageFlagBits;
using vkBU = vk::BufferUsageFlagBits;
using vkMP = vk::MemoryPropertyFlagBits;

void App::run()
{
    initVulkan();
    prepare();
    mainLoop();
}

void App::initVulkan()
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
    vk::PhysicalDeviceBufferDeviceAddressFeatures bufferDeviceAddressFeature{ true };
    vk::PhysicalDeviceRayTracingPipelineFeaturesKHR rayTracingPipelineFeaturesKHR{ true };
    vk::PhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeaturesKHR{ true };
    bufferDeviceAddressFeature.setPNext(&rayTracingPipelineFeaturesKHR);
    rayTracingPipelineFeaturesKHR.setPNext(&accelerationStructureFeaturesKHR);
    void* deviceCreatePNext = &bufferDeviceAddressFeature;

    // Init vulkan context
    context.initialize(VK_API_VERSION_1_2, true, width, height,
                       deviceExtensions, deviceCreatePNext);

    resourceManager.initialize(context);
    rtShaderManager.initialize(context);
    rtPipeline.initialize(context);
}

void App::mainLoop()
{
    while (context.running()) {
        context.pollEvents();
        draw();
    }
    context.getDevice().waitIdle();
}

void App::prepare()
{
    // Add resources
    renderImage = &resourceManager.addStorageImage(
        context.getSwapchain().getExtent(),
        context.getSwapchain().getFormat(),
        vkIU::eStorage | vkIU::eTransferSrc | vkIU::eTransferDst,
        vkIL::eGeneral);

    vertices.push_back(Vertex{ {0.0, 1.0, 0.0} });
    vertices.push_back(Vertex{ {1.0, 0.0, 0.0} });
    vertices.push_back(Vertex{ {-1.0, 0.0, 0.0} });
    vertexBuffer = &resourceManager.addStorageBuffer(
        sizeof(Vertex) * vertices.size(),
        vkBU::eAccelerationStructureBuildInputReadOnlyKHR |
        vkBU::eStorageBuffer | vkBU::eShaderDeviceAddress,
        vkMP::eHostVisible | vkMP::eHostCoherent,
        vertices.data());

    indices = { 0, 1, 2 };
    indexBuffer = &resourceManager.addStorageBuffer(
        sizeof(Index) * indices.size(),
        vkBU::eAccelerationStructureBuildInputReadOnlyKHR |
        vkBU::eStorageBuffer | vkBU::eShaderDeviceAddress,
        vkMP::eHostVisible | vkMP::eHostCoherent,
        indices.data());

    bottomLevelAS.initialize(context.getDevice(), context.getPhysicalDevice(),
                             vertices, *vertexBuffer,
                             indices, *indexBuffer);

    resourceManager.prepare();

    // Load shaders
    rtShaderManager.addRaygenShader("shader/spv/raygen.rgen.spv");
    rtShaderManager.addChitShader("shader/spv/closesthit.rchit.spv");
    rtShaderManager.addMissShader("shader/spv/miss.rmiss.spv");

    rtPipeline.prepare(rtShaderManager, resourceManager);
    rtShaderManager.initShaderBindingTable(rtPipeline);

    // Build draw command buffers
    drawCommandBuffers = context.getSwapchain().allocateDrawComamndBuffers();
    for (int32_t i = 0; i < drawCommandBuffers.size(); ++i) {
        const auto& cmdBuf = drawCommandBuffers[i].get();
        const auto& swapchainImage = context.getSwapchain().getImages()[i];
        const auto& bindPoint = vk::PipelineBindPoint::eRayTracingKHR;
        const auto& extent = context.getSwapchain().getExtent();

        cmdBuf.begin(vk::CommandBufferBeginInfo{});

        cmdBuf.bindPipeline(bindPoint, rtPipeline.get());

        cmdBuf.bindDescriptorSets(bindPoint,
                                  rtPipeline.getLayout(), 0,
                                  resourceManager.getDescSet(), nullptr);

        cmdBuf.traceRaysKHR(rtShaderManager.getRaygenRegion(),
                            rtShaderManager.getMissRegion(),
                            rtShaderManager.getHitRegion(),
                            {}, width, height, 1);

        Image::transitionImageLayout(cmdBuf, renderImage->get(),
                                     vkIL::eUndefined, vkIL::eTransferSrcOptimal);

        Image::transitionImageLayout(cmdBuf, swapchainImage,
                                     vkIL::eUndefined, vkIL::eTransferDstOptimal);

        Image::copyImage(cmdBuf, renderImage->get(), swapchainImage, extent);

        Image::transitionImageLayout(cmdBuf, renderImage->get(),
                                     vkIL::eTransferSrcOptimal, vkIL::eGeneral);

        Image::transitionImageLayout(cmdBuf, swapchainImage,
                                     vkIL::eTransferDstOptimal, vkIL::ePresentSrcKHR);

        cmdBuf.end();
    }
}

void App::draw()
{
    // Begin
    Swapchain::FrameInfo frameInfo = context.getSwapchain().beginFrame();
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
