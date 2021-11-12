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
    vkt::Window window{ width, height, "Window" };

    // Init vulkan context
    vkt::ContextCreateInfo contextInfo{ .enableValidationLayer = true };
    vkt::Context context{ contextInfo, window };

    // Init swapchain
    vkt::Swapchain swapchain{ context, width, height };

    // Create render image
    vkt::Image renderImage{ context, swapchain.getExtent(), swapchain.getFormat(),
                           vkIU::eStorage | vkIU::eTransferSrc | vkIU::eTransferDst };
    renderImage.createImageView();
    renderImage.transitionLayout(vk::ImageLayout::eGeneral);

    // Create desc set
    vkt::DescriptorPool descPool{ context, 1, { {vk::DescriptorType::eStorageImage, 10} } };

    vk::DescriptorSetLayoutBinding imageBinding;
    imageBinding.setBinding(0);
    imageBinding.setDescriptorCount(1);
    imageBinding.setDescriptorType(vk::DescriptorType::eStorageImage);
    imageBinding.setStageFlags(vk::ShaderStageFlagBits::eCompute);

    vk::UniqueDescriptorSetLayout descSetLayout = descPool.createDescSetLayout({ imageBinding });
    vkt::DescriptorSet descSet = descPool.createDescSet(*descSetLayout);
    descSet.update(renderImage, imageBinding);

    // Load shaders
    vkt::ShaderModule shaderModule{ context, shader, vk::ShaderStageFlagBits::eCompute };

    // Create pipeline
    vkt::ComputePipeline pipeline{ context, *descSetLayout, shaderModule };

    // Build draw command buffers
    auto drawCommandBuffers = swapchain.allocateDrawComamndBuffers();
    for (int32_t i = 0; i < drawCommandBuffers.size(); ++i) {
        const auto& cmdBuf = drawCommandBuffers[i].get();
        const auto& swapchainImage = swapchain.getImages()[i];
        const auto& extent = swapchain.getExtent();

        cmdBuf.begin(vk::CommandBufferBeginInfo{});

        pipeline.bind(cmdBuf);

        descSet.bind(cmdBuf, pipeline);

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
