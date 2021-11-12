#pragma once
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>
#include "ShaderModule.hpp"

namespace vkt
{
    class Context;
    class DescriptorSetLayout;

    class Pipeline
    {
    public:
        Pipeline() = default;
        Pipeline(const Pipeline&) = delete;
        Pipeline(Pipeline&&) = default;
        Pipeline& operator=(const Pipeline&) = delete;
        Pipeline& operator=(Pipeline&&) = default;

        virtual vk::PipelineBindPoint getBindPoint() const = 0;

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
                        const DescriptorSetLayout& descSetLayout,
                        const ComputeShaderModule& shaderModule);

        vk::PipelineBindPoint getBindPoint() const override
        {
            return vk::PipelineBindPoint::eCompute;
        }
    };
}
