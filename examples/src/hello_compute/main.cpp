#include "vktiny/vktiny.hpp"
#include <iostream>

const std::string computeShaderText = R"(
#version 460
layout(local_size_x = 1, local_size_y = 1) in;
//layout(binding = 0, rgba8) uniform image2D renderImage;

void main()
{
    //vec3 color = vec3(gl_GlobalInvocationID.xyz) / gl_NumWorkGroups.xyz;
	//imageStore(renderImage, ivec2(gl_GlobalInvocationID.xy), vec4(color, 1));
}
)";

class App
{
public:
    App()
    {
        vkt::ContextCreateInfo contextInfo;
        contextInfo.setDebug(true);
        contextInfo.setWindowSize(1280, 720);
        context.initialize(contextInfo);
        storageImage.initialize(context, { 1280, 720 }, vk::ImageUsageFlagBits::eStorage);

        vk::DescriptorSetLayoutBinding binding;
        binding.setBinding(0);
        binding.setDescriptorType(vk::DescriptorType::eStorageImage);
        binding.setDescriptorCount(1);
        binding.setStageFlags(vk::ShaderStageFlagBits::eCompute);

        descSetLayout.initialize(context, { binding });
        pipeline.initialize(context, descSetLayout, computeShaderText);
        descPool.initialize(context, 1, { {vk::DescriptorType::eStorageImage, 10} });
        descSet.initialize(context, descPool, descSetLayout);
        // update?

        //drawCmdBufs = context.allocateComputeCommandBuffers(3);
    }

    void run()
    {
        while (!context.shouldTerminate()) {
            context.pollEvents();
        }
    }

private:
    vkt::Context context;
    vkt::Image storageImage;
    vkt::DescriptorSetLayout descSetLayout;
    vkt::ComputePipeline pipeline;
    vkt::DescriptorPool descPool;
    vkt::DescriptorSet descSet;
    //vk::raii::CommandBuffers drawCmdBufs;
};

int main()
{
    try {
        App app;
        app.run();
        std::cout << "OK!" << std::endl;
    } catch (std::exception exception) {
        std::cerr << exception.what() << std::endl;
    }
}
