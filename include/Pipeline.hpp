#pragma once
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>
#include "Device.hpp"
#include "ResourceManager.hpp"
#include "ShaderManager.hpp"
#include "Context.hpp"

class Pipeline
{
public:
    void initialize(const Context& context);

    vk::Pipeline get() const { return pipeline.get(); }
    vk::PipelineLayout getLayout() const { return layout.get(); }

protected:
    vk::Device device;
    vk::UniquePipeline pipeline;
    vk::UniquePipelineLayout layout;
};

class RayTracingPipeline : public Pipeline
{
public:
    void setMaxRecursion(uint32_t maxRecursion) { this->maxRecursion = maxRecursion; }

    void prepare(const RayTracingShaderManager& shaderManager,
                 const ResourceManager& resourceManager);

private:
    uint32_t maxRecursion = 4;
};

class RasterPipeline : public Pipeline
{
    void prepare()
    {

    }
};
