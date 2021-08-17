#include "Pipeline.hpp"

void Pipeline::initialize(const Context& context)
{
    this->device = context.getDevice().get();
}

void RayTracingPipeline::prepare(const RayTracingShaderManager& shaderManager,
                                 const DescriptorManager& descriptorManager)
{
    layout = device.createPipelineLayoutUnique({ {}, descriptorManager.getDescSetLayout() });

    vk::RayTracingPipelineCreateInfoKHR createInfo;
    createInfo.setStages(shaderManager.getStages());
    createInfo.setGroups(shaderManager.getRtGroups());
    createInfo.setMaxPipelineRayRecursionDepth(maxRecursion);
    createInfo.setLayout(*layout);
    auto res = device.createRayTracingPipelineKHRUnique(nullptr, nullptr, createInfo);
    if (res.result == vk::Result::eSuccess) {
        pipeline = std::move(res.value);
        return;
    }
    throw std::runtime_error("failed to create ray tracing pipeline.");
}
