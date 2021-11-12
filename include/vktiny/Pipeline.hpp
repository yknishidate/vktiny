#pragma once
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>
#include "vktiny/DescriptorManager.hpp"
#include "vktiny/ShaderManager.hpp"
#include "vktiny/Context.hpp"

namespace vkt
{
    class Pipeline
    {
    public:
        Pipeline() = default;
        Pipeline(const Pipeline &) = delete;
        Pipeline(Pipeline &&) = default;
        Pipeline &operator=(const Pipeline &) = delete;
        Pipeline &operator=(Pipeline &&) = default;

        virtual void initialize(const Context &context);

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
        void initialize(const Context &context) override;

        void setMaxRecursion(uint32_t maxRecursion) { this->maxRecursion = maxRecursion; }

        void prepare(const DescriptorManager &descriptorManager);

        // shader mamager
        void addRaygenShader(const std::string filepath)
        {
            rtShaderManager.addRaygenShader(filepath);
        }
        void addMissShader(const std::string filepath)
        {
            rtShaderManager.addMissShader(filepath);
        }
        void addChitShader(const std::string filepath)
        {
            rtShaderManager.addChitShader(filepath);
        }
        void addAhitShader(const std::string filepath)
        {
            rtShaderManager.addAhitShader(filepath);
        }
        void addChitAndAhitShader(const std::string chitFilepath, const std::string ahitFilepath)
        {
            rtShaderManager.addChitAndAhitShader(chitFilepath, ahitFilepath);
        }

        const auto &getRtGroups() const { return rtShaderManager.getRtGroups(); }
        const auto &getRaygenRegion() const { return rtShaderManager.getRaygenRegion(); }
        const auto &getMissRegion() const { return rtShaderManager.getMissRegion(); }
        const auto &getHitRegion() const { return rtShaderManager.getHitRegion(); }

    private:
        RayTracingShaderManager rtShaderManager;
        uint32_t maxRecursion = 4;
    };

    class RasterPipeline : public Pipeline
    {
        void prepare()
        {
        }
    };

    class ComputePipeline : public Pipeline
    {
    public:
        void initialize(const Context &context) override
        {
            Pipeline::initialize(context);
            shaderManager.initialize(context);
        }

        void addComputeShader(const std::string filepath)
        {
            shaderManager.addShader(filepath, vk::ShaderStageFlagBits::eCompute);
        }

        void addComputeShaderFromText(const std::string &shaderText)
        {
            shaderManager.addShaderFromText(shaderText, vk::ShaderStageFlagBits::eCompute);
        }

        void prepare(const DescriptorManager &descriptorManager)
        {
            layout = device.createPipelineLayoutUnique({{}, descriptorManager.getDescSetLayout()});

            vk::ComputePipelineCreateInfo createInfo;
            createInfo.setStage(shaderManager.getStages().front());
            createInfo.setLayout(*layout);
            auto res = device.createComputePipelineUnique(nullptr, createInfo);
            if (res.result == vk::Result::eSuccess)
            {
                pipeline = std::move(res.value);
            }
            else
            {
                throw std::runtime_error("failed to create compute pipeline.");
            }
        }

    private:
        ShaderManager shaderManager;
    };
}
