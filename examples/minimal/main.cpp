#include <iostream>
#include "Context.hpp"
#include "ResourceManager.hpp"
#include "ShaderManager.hpp"
#include "Pipeline.hpp"

using vkIU = vk::ImageUsageFlagBits;
using vkIL = vk::ImageLayout;

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
    Context context;
    context.initialize(VK_API_VERSION_1_2, true, width, height,
                       deviceExtensions, deviceCreatePNext);

    ResourceManager resourceManager;
    resourceManager.initialize(context);

    RayTracingShaderManager rtShaderManager;
    rtShaderManager.initialize(context);

    RayTracingPipeline rtPipeline;
    rtPipeline.initialize(context);

    // Create render image
    Image& renderImage = resourceManager.addStorageImage(
        context.getSwapchain().getExtent(),
        context.getSwapchain().getFormat(),
        vkIU::eStorage | vkIU::eTransferSrc | vkIU::eTransferDst,
        vkIL::eGeneral);

    // Load shaders
    rtShaderManager.addRaygenShader("shader/spv/raygen.rgen.spv");
    rtShaderManager.addChitShader("shader/spv/closesthit.rchit.spv");
    rtShaderManager.addMissShader("shader/spv/miss.rmiss.spv");

    resourceManager.prepare();
    rtPipeline.prepare(rtShaderManager, resourceManager);
    rtShaderManager.initShaderBindingTable(rtPipeline);

    // Build draw command buffers
    auto drawCommandBuffers = context.getSwapchain().allocateDrawComamndBuffers();
    for (int32_t i = 0; i < drawCommandBuffers.size(); ++i) {
        const auto& cmdBuf = drawCommandBuffers[i].get();
        const auto& swapchainImage = context.getSwapchain().getImages()[i];
        const auto& bindPoint = vk::PipelineBindPoint::eRayTracingKHR;
        const auto& extent = context.getSwapchain().getExtent();

        cmdBuf.begin(vk::CommandBufferBeginInfo{});

        cmdBuf.bindPipeline(bindPoint, rtPipeline.get());

        cmdBuf.bindDescriptorSets(bindPoint, rtPipeline.getLayout(), 0,
                                  resourceManager.getDescSet(), nullptr);

        cmdBuf.traceRaysKHR(rtShaderManager.getRaygenRegion(),
                            rtShaderManager.getMissRegion(),
                            rtShaderManager.getHitRegion(),
                            {}, width, height, 1);

        Image::transitionImageLayout(cmdBuf, renderImage.get(),
                                     vkIL::eUndefined, vkIL::eTransferSrcOptimal);
        Image::transitionImageLayout(cmdBuf, swapchainImage,
                                     vkIL::eUndefined, vkIL::eTransferDstOptimal);

        Image::copyImage(cmdBuf, renderImage.get(), swapchainImage, extent);

        Image::transitionImageLayout(cmdBuf, renderImage.get(),
                                     vkIL::eTransferSrcOptimal, vkIL::eGeneral);
        Image::transitionImageLayout(cmdBuf, swapchainImage,
                                     vkIL::eTransferDstOptimal, vkIL::ePresentSrcKHR);
        cmdBuf.end();
    }

    while (context.running()) {
        context.pollEvents();

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
    context.getDevice().waitIdle();
}
