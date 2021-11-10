#pragma once
#include <fstream>
#include "Context.hpp"
#include "DescriptorSetLayout.hpp"

namespace vkt
{
    std::vector<char> readFile(const std::string& filename);
    std::vector<unsigned int> compileToSPV(vk::ShaderStageFlagBits shaderType,
                                           const std::string& glslShader);

    class Pipeline
    {
    public:
        Pipeline() = default;
        Pipeline(const Pipeline&) = delete;
        Pipeline& operator = (const Pipeline&) = delete;

    protected:
        vk::raii::Pipeline pipeline = nullptr;
        vk::raii::PipelineLayout layout = nullptr;
    };

    class ComputePipeline : public Pipeline
    {
    public:
        ComputePipeline() = default;

        void initialize(const Context& context,
                        const DescriptorSetLayout& descSetLayout,
                        const std::string& shaderText)
        {
            layout = vk::raii::PipelineLayout(context.getDevice(), { {}, *descSetLayout.get() });

            auto shaderSPV = compileToSPV(vk::ShaderStageFlagBits::eCompute, shaderText);
            computeShaderModule = vk::raii::ShaderModule(context.getDevice(), { {}, shaderSPV });

            vk::PipelineShaderStageCreateInfo stageInfo;
            stageInfo.setStage(vk::ShaderStageFlagBits::eCompute);
            stageInfo.setModule(*computeShaderModule);
            stageInfo.setPName("main");

            vk::ComputePipelineCreateInfo pipelineInfo;
            pipelineInfo.setStage(stageInfo);
            pipelineInfo.setLayout(*layout);
            pipeline = vk::raii::Pipeline(context.getDevice(), nullptr, pipelineInfo);
        }

    private:
        vk::raii::ShaderModule computeShaderModule = nullptr;
    };
}
