#pragma once
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>
#include "vktiny/ShaderModule.hpp"
#include "vktiny/Context.hpp"

namespace vkt
{
    class Pipeline
    {
    public:
        Pipeline() = default;
        Pipeline(const Pipeline&) = delete;
        Pipeline(Pipeline&&) = default;
        Pipeline& operator=(const Pipeline&) = delete;
        Pipeline& operator=(Pipeline&&) = default;

        virtual vk::PipelineBindPoint getBindPoint() const = 0;

        void bind(vk::CommandBuffer commandBuffer) const
        {
            commandBuffer.bindPipeline(getBindPoint(), *pipeline);
        }

        vk::Pipeline get() const { return pipeline.get(); }
        vk::PipelineLayout getLayout() const { return layout.get(); }

    protected:
        vk::UniquePipeline pipeline;
        vk::UniquePipelineLayout layout;
    };

    class ComputePipeline : public Pipeline
    {
    public:
        ComputePipeline(const Context& context,
                        vk::DescriptorSetLayout descSetLayout,
                        const ShaderModule& shaderModule)
        {
            layout = context.getDevice().createPipelineLayoutUnique({ {}, descSetLayout });

            vk::ComputePipelineCreateInfo pipelineInfo;
            pipelineInfo.setStage(shaderModule.getStageInfo());
            pipelineInfo.setLayout(*layout);
            pipeline = context.getDevice().createComputePipelineUnique(nullptr, pipelineInfo);
        }

        vk::PipelineBindPoint getBindPoint() const override
        {
            return vk::PipelineBindPoint::eCompute;
        }
    };
}
