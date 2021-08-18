#include "vktiny/Pipeline.hpp"

namespace vkt
{
    void Pipeline::initialize(const Context& context)
    {
        this->device = context.getVkDevice();
    }

    void RayTracingPipeline::initialize(const Context& context)
    {
        Pipeline::initialize(context);
        rtShaderManager.initialize(context);
    }

    void RayTracingPipeline::prepare(const DescriptorManager& descriptorManager)
    {
        layout = device.createPipelineLayoutUnique({ {}, descriptorManager.getDescSetLayout() });

        vk::RayTracingPipelineCreateInfoKHR createInfo;
        createInfo.setStages(rtShaderManager.getStages());
        createInfo.setGroups(rtShaderManager.getRtGroups());
        createInfo.setMaxPipelineRayRecursionDepth(maxRecursion);
        createInfo.setLayout(*layout);
        auto res = device.createRayTracingPipelineKHRUnique(nullptr, nullptr, createInfo);
        if (res.result != vk::Result::eSuccess) {
            throw std::runtime_error("failed to create ray tracing pipeline.");
        }
        pipeline = std::move(res.value);
        rtShaderManager.initShaderBindingTable(*this);
    }
}
