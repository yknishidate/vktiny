#include "vktiny/vktiny.hpp"

using vkIL = vk::ImageLayout;
using vkIU = vk::ImageUsageFlagBits;

const std::string shader = R"(
#version 460
layout(local_size_x = 1, local_size_y = 1) in;
layout(binding = 0, rgba8) uniform image2D renderImage;

void main()
{
    vec3 color = vec3(gl_GlobalInvocationID.xyz) / gl_NumWorkGroups.xyz;
	imageStore(renderImage, ivec2(gl_GlobalInvocationID.xy), vec4(color, 1));
}
)";

int main()
{
    int width = 1280;
    int height = 720;

    // Init window
    vkt::Window window;
    window.initialize(width, height, "Window");

    // Init vulkan context
    vkt::ContextCreateInfo contextInfo;
    contextInfo.apiMinorVersion = 2;
    contextInfo.enableValidationLayer = true;
    contextInfo.deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    vkt::Context context;
    context.initialize(contextInfo, window);

    // Init swapchain
    vkt::Swapchain swapchain;
    swapchain.initialize(context, width, height);

    // Create render image
    vkt::Image renderImage;
    renderImage.initialize(context, swapchain.getExtent(), swapchain.getFormat(),
                           vkIU::eStorage | vkIU::eTransferSrc | vkIU::eTransferDst);
    renderImage.createImageView();
    renderImage.transitionLayout(vk::ImageLayout::eGeneral);

    // Add descriptor bindings
    vkt::DescriptorManager descManager;
    descManager.initialize(context);
    descManager.addStorageImage(renderImage, 0);
    descManager.prepare();

    // Load shaders
    vkt::ShaderModule shaderModule;
    shaderModule.initialize(context, shader, vk::ShaderStageFlagBits::eCompute);

    // Create pipeline
    vkt::ComputePipeline pipeline;
    pipeline.initialize(context, descManager.getDescSetLayout(), shaderModule);

    // Build draw command buffers
    auto drawCommandBuffers = swapchain.allocateDrawComamndBuffers();
    for (int32_t i = 0; i < drawCommandBuffers.size(); ++i) {
        const auto& cmdBuf = drawCommandBuffers[i].get();
        const auto& swapchainImage = swapchain.getImages()[i];
        const auto& extent = swapchain.getExtent();

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

    while (!window.shouldClose()) {
        window.pollEvents();

        // Begin
        vkt::FrameInfo frameInfo = swapchain.beginFrame();
        const auto& cmdBuf = *drawCommandBuffers[frameInfo.imageIndex];

        // Render
        vk::PipelineStageFlags waitStage{ vk::PipelineStageFlagBits::eComputeShader };
        vk::SubmitInfo submitInfo;
        submitInfo.setWaitSemaphores(frameInfo.imageAvailableSemaphore);
        submitInfo.setWaitDstStageMask(waitStage);
        submitInfo.setCommandBuffers(cmdBuf);
        submitInfo.setSignalSemaphores(frameInfo.renderFinishedSemaphore);
        context.getGraphicsQueue().submit(submitInfo, frameInfo.inFlightFence);

        // End
        swapchain.endFrame(frameInfo.imageIndex);
    }
    context.getDevice().waitIdle();
}
