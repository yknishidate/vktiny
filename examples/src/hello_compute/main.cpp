#include "vktiny/vktiny.hpp"
#include <iostream>
#include <SPIRV/GlslangToSpv.h>

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
        context.initialize(contextInfo);

        glslang::InitializeProcess();
        //vk::raii::ShaderModule vertexShaderModule =
            //vk::raii::su::makeShaderModule(device, vk::ShaderStageFlagBits::eVertex, vertexShaderText_PC_C);
        glslang::FinalizeProcess();
    }

    void run()
    {
        while (!context.shouldTerminate()) {
            context.pollEvents();
        }
    }

private:
    vkt::Context context;
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
