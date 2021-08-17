#include <iostream>
#include "Context.hpp"
#include "DescriptorManager.hpp"
#include "ShaderManager.hpp"
#include "Pipeline.hpp"
#include "AccelStruct.hpp"

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
    Context context;
    context.initialize(VK_API_VERSION_1_2, true, width, height,
                       deviceExtensions, deviceCreatePNext);

    DescriptorManager descManager;
    descManager.initialize(context);

    RayTracingShaderManager rtShaderManager;
    rtShaderManager.initialize(context);

    RayTracingPipeline rtPipeline;
    rtPipeline.initialize(context);

    // Create render image
    Image renderImage;
    renderImage.initialize(context.getDevice(), context.getPhysicalDevice(),
                           context.getSwapchain().getExtent(),
                           context.getSwapchain().getFormat(),
                           vkIU::eStorage | vkIU::eTransferSrc | vkIU::eTransferDst);
    renderImage.createImageView();
    renderImage.transitionImageLayout(vk::ImageLayout::eGeneral);

    // Create vertices and indices
    std::vector<Vertex> vertices;
    vertices.push_back(Vertex{ { 0.0, -0.3, 0.0} });
    vertices.push_back(Vertex{ { 0.3,  0.3, 0.0} });
    vertices.push_back(Vertex{ {-0.3,  0.3, 0.0} });
    std::vector<Index> indices{ 0, 1, 2 };

    // Create vertex buffer
    Buffer vertexBuffer;
    vertexBuffer.initialize(context.getDevice(), context.getPhysicalDevice(),
                            sizeof(Vertex) * vertices.size(),
                            vkBU::eAccelerationStructureBuildInputReadOnlyKHR |
                            vkBU::eStorageBuffer | vkBU::eShaderDeviceAddress,
                            vkMP::eHostVisible | vkMP::eHostCoherent);
    vertexBuffer.copy(vertices.data());

    // Create index buffer
    Buffer indexBuffer;
    indexBuffer.initialize(context.getDevice(), context.getPhysicalDevice(),
                           sizeof(Index) * indices.size(),
                           vkBU::eAccelerationStructureBuildInputReadOnlyKHR |
                           vkBU::eStorageBuffer | vkBU::eShaderDeviceAddress,
                           vkMP::eHostVisible | vkMP::eHostCoherent);
    indexBuffer.copy(indices.data());

    // Create bottom level accel struct
    BottomLevelAccelStruct bottomLevelAS;
    bottomLevelAS.initialize(context.getDevice(), context.getPhysicalDevice(),
                             vertices, vertexBuffer,
                             indices, indexBuffer);

    TopLevelAccelStruct topLevelAS;
    topLevelAS.initialize(context.getDevice(), context.getPhysicalDevice(), bottomLevelAS);

    // Add descriptor binding
    descManager.addStorageImage(renderImage, 0);
    descManager.addTopLevelAccelStruct(topLevelAS, 3);
    descManager.prepare();

    // Load shaders
    rtShaderManager.addRaygenShader("shader/spv/raygen.rgen.spv");
    rtShaderManager.addMissShader("shader/spv/miss.rmiss.spv");
    rtShaderManager.addChitShader("shader/spv/closesthit.rchit.spv");

    rtPipeline.prepare(rtShaderManager, descManager);
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
                                  descManager.getDescSet(), nullptr);

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
