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

    vkt::Window window{ width, height, "Window" };

    vkt::ContextCreateInfo contextInfo{ .enableValidationLayer = true };
    vkt::Context context{ contextInfo, window };

    vkt::Swapchain swapchain{ context, width, height };

    // Create resources
    vkt::Image renderImage{ context, swapchain.getExtent(), swapchain.getFormat(),
                           vkIU::eStorage | vkIU::eTransferSrc };
    renderImage.createImageView();
    renderImage.transitionLayout(vk::ImageLayout::eGeneral);

    // Create descriptors
    vkt::DescriptorPool descPool{ context, 1, { {vk::DescriptorType::eStorageImage, 10} } };

    vk::DescriptorSetLayoutBinding imageBinding;
    imageBinding.setBinding(0);
    imageBinding.setDescriptorCount(1);
    imageBinding.setDescriptorType(vk::DescriptorType::eStorageImage);
    imageBinding.setStageFlags(vk::ShaderStageFlagBits::eCompute);

    vkt::DescriptorSetLayout descSetLayout{ context, { imageBinding } };
    vkt::DescriptorSet descSet{ context, descPool, descSetLayout };
    descSet.update(renderImage, imageBinding);

    // Create pipeline
    vkt::ComputeShaderModule shaderModule{ context, shader };
    vkt::ComputePipeline pipeline{ context, descSetLayout, shaderModule };

    size_t bufferCount = swapchain.getImagesSize();
    auto drawCommandBuffers = context.allocateGraphicsCommandBuffers(bufferCount);
    for (int32_t i = 0; i < bufferCount; ++i) {
        vkt::CommandBuffer& cmdBuf = drawCommandBuffers[i];
        const auto& swapchainImage = swapchain.getImages()[i];
        const auto& extent = swapchain.getExtent();

        cmdBuf.begin();
        cmdBuf.bindPipeline(pipeline);
        cmdBuf.bindDescriptorSets(descSet, pipeline);
        cmdBuf.dispatch(width, height, 1);

        cmdBuf.transitionImageLayout(renderImage.get(), vkIL::eUndefined, vkIL::eTransferSrcOptimal);
        cmdBuf.transitionImageLayout(swapchainImage, vkIL::eUndefined, vkIL::eTransferDstOptimal);
        cmdBuf.copyImage(renderImage.get(), swapchainImage, extent);
        cmdBuf.transitionImageLayout(renderImage.get(), vkIL::eTransferSrcOptimal, vkIL::eGeneral);
        cmdBuf.transitionImageLayout(swapchainImage, vkIL::eTransferDstOptimal, vkIL::ePresentSrcKHR);

        cmdBuf.end();
    }

    while (!window.shouldClose()) {
        window.pollEvents();

        // Begin
        vkt::FrameInfo frameInfo = swapchain.beginFrame();
        vk::CommandBuffer cmdBuf = drawCommandBuffers[frameInfo.imageIndex].get();

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
