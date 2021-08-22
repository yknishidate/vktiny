#include "vktiny/vktiny.hpp"

using vkIL = vk::ImageLayout;
using vkIU = vk::ImageUsageFlagBits;

int main()
{
    int width = 1280;
    int height = 720;

    // Add device extensions
    std::vector<const char*> deviceExtensions;
    deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    // Init vulkan context
    vkt::Context context;
    context.initialize(VK_API_VERSION_1_2, true, width, height, deviceExtensions);
    context.setWindowIcon("asset/vulkan.png");

    // Create render image
    vkt::Image renderImage;
    renderImage.initialize(context,
                           context.getSwapchain().getExtent(),
                           context.getSwapchain().getFormat(),
                           vkIU::eStorage | vkIU::eTransferSrc | vkIU::eTransferDst);
    renderImage.createImageView();
    renderImage.transitionLayout(vk::ImageLayout::eGeneral);

    // Add descriptor bindings
    vkt::DescriptorManager descManager;
    descManager.initialize(context);
    descManager.addStorageImage(renderImage, 0);
    descManager.prepare();

    // Load shaders
    vkt::ComputePipeline pipeline;
    pipeline.initialize(context);
    pipeline.addComputeShader("shader/hello_compute/spv/compute.comp.spv");
    pipeline.prepare(descManager);

    // Build draw command buffers
    auto drawCommandBuffers = context.getSwapchain().allocateDrawComamndBuffers();
    for (int32_t i = 0; i < drawCommandBuffers.size(); ++i) {
        const auto& cmdBuf = drawCommandBuffers[i].get();
        const auto& swapchainImage = context.getSwapchain().getImages()[i];
        const auto& extent = context.getSwapchain().getExtent();

        cmdBuf.begin(vk::CommandBufferBeginInfo{});

        cmdBuf.bindPipeline(vk::PipelineBindPoint::eCompute, pipeline.get());

        cmdBuf.bindDescriptorSets(vk::PipelineBindPoint::eCompute, pipeline.getLayout(), 0,
                                  descManager.getDescSet(), nullptr);

        cmdBuf.dispatch(width, height, 1);

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
        vk::PipelineStageFlags waitStage{ vk::PipelineStageFlagBits::eComputeShader };
        vk::SubmitInfo submitInfo;
        submitInfo.setWaitSemaphores(frameInfo.imageAvailableSemaphore);
        submitInfo.setWaitDstStageMask(waitStage);
        submitInfo.setCommandBuffers(cmdBuf);
        submitInfo.setSignalSemaphores(frameInfo.renderFinishedSemaphore);
        context.getDevice().getGraphicsQueue().submit(submitInfo, frameInfo.inFlightFence);

        // End
        context.getSwapchain().endFrame(frameInfo.imageIndex);
    }
    context.getDevice().waitIdle();
}
