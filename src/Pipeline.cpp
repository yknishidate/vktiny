#include "vktiny/Pipeline.hpp"
#include "vktiny/Context.hpp"

vkt::ComputePipeline::ComputePipeline(const Context& context,
                                      vk::DescriptorSetLayout descSetLayout,
                                      const ShaderModule& shaderModule)
{
    layout = context.getDevice().createPipelineLayoutUnique({ {}, descSetLayout });

    vk::ComputePipelineCreateInfo pipelineInfo;
    pipelineInfo.setStage(shaderModule.getStageInfo());
    pipelineInfo.setLayout(*layout);
    pipeline = context.getDevice().createComputePipelineUnique(nullptr, pipelineInfo);
}
