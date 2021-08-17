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
    void initialize(const Context& context)
    {
        this->device = context.getDevice().get();
    }

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
    void setMaxRecursion(uint32_t maxRecursion)
    {
        this->maxRecursion = maxRecursion;
    }

    void prepare(const RayTracingShaderManager& shaderManager,
                 const ResourceManager& resourceManager)
    {
        layout = device.createPipelineLayoutUnique({ {}, resourceManager.getDescSetLayout() });

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

private:
    uint32_t maxRecursion = 4;
};

class RasterPipeline : public Pipeline
{
    void prepare()
    {

    }
};
