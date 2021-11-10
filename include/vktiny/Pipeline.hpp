#pragma once
#include "Context.hpp"
#include <fstream>

namespace vkt
{

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

        //void initialize(const Context& context,
        //                const std::vector<vk::DescriptorSetLayoutBinding>& bindings,
        //                vk::raii::ShaderModule shaderModule)
        //{
        //    vk::raii::DescriptorSetLayout descSetLayout(context.getDevice(), { {}, bindings });
        //    layout = vk::raii::PipelineLayout(context.getDevice(), { {}, *descSetLayout });

        //    vk::PipelineShaderStageCreateInfo stageInfo;
        //    stageInfo.setStage(vk::ShaderStageFlagBits::eCompute);
        //    stageInfo.setModule(*shaderModule);
        //    stageInfo.setPName("main");

        //    vk::ComputePipelineCreateInfo pipelineInfo;
        //    pipelineInfo.setStage(stageInfo);
        //    pipelineInfo.setLayout(*layout);
        //    pipeline = vk::raii::Pipeline(context.getDevice(), nullptr, pipelineInfo);
        //}

        void initialize(const Context& context,
                        const std::vector<vk::DescriptorSetLayoutBinding>& bindings,
                        const std::string& shaderText)
        {
            vk::raii::DescriptorSetLayout descSetLayout(context.getDevice(), { {}, bindings });
            layout = vk::raii::PipelineLayout(context.getDevice(), { {}, *descSetLayout });

            auto shaderSPV = GLSLtoSPV(vk::ShaderStageFlagBits::eCompute, shaderText);
            //computeShaderModule = vk::raii::ShaderModule(context.getDevice(), { {}, shaderSPV });

            //vk::PipelineShaderStageCreateInfo stageInfo;
            //stageInfo.setStage(vk::ShaderStageFlagBits::eCompute);
            //stageInfo.setModule(*computeShaderModule);
            //stageInfo.setPName("main");

            //vk::ComputePipelineCreateInfo pipelineInfo;
            //pipelineInfo.setStage(stageInfo);
            //pipelineInfo.setLayout(*layout);
            //pipeline = vk::raii::Pipeline(context.getDevice(), nullptr, pipelineInfo);
        }

        //void initialize(const Context& context,
        //                const std::vector<vk::DescriptorSetLayoutBinding>& bindings,
        //                vk::raii::ShaderModule shaderModule)
        //{
        //    vk::raii::DescriptorSetLayout descSetLayout(context.getDevice(), { {}, bindings });
        //    layout = vk::raii::PipelineLayout(context.getDevice(), { {}, *descSetLayout });

        //    vk::PipelineShaderStageCreateInfo stageInfo;
        //    stageInfo.setStage(vk::ShaderStageFlagBits::eCompute);
        //    stageInfo.setModule(*shaderModule);
        //    stageInfo.setPName("main");

        //    vk::ComputePipelineCreateInfo pipelineInfo;
        //    pipelineInfo.setStage(stageInfo);
        //    pipelineInfo.setLayout(*layout);
        //    pipeline = vk::raii::Pipeline(context.getDevice(), nullptr, pipelineInfo);
        //}

        std::vector<unsigned int> GLSLtoSPV(vk::ShaderStageFlagBits shaderType,
                                            const std::string& glslShader);

    private:
        vk::raii::ShaderModule computeShaderModule = nullptr;
    };
}
