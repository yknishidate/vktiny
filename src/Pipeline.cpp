#include "vktiny/Pipeline.hpp"
#include "vktiny/Context.hpp"
#include "vktiny/DescriptorSetLayout.hpp"

vkt::ComputePipeline::ComputePipeline(const Context& context,
                                      const DescriptorSetLayout& descSetLayout,
                                      const ComputeShaderModule& shaderModule)
{
    vk::DescriptorSetLayout setLayout = descSetLayout.get();
    layout = context.getDevice().createPipelineLayoutUnique({ {}, setLayout });

    vk::ComputePipelineCreateInfo pipelineInfo;
    pipelineInfo.setStage(shaderModule.getStageInfo());
    pipelineInfo.setLayout(*layout);
    pipeline = context.getDevice().createComputePipelineUnique(nullptr, pipelineInfo);
}
