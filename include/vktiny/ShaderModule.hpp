#pragma once
#include "Context.hpp"

namespace vkt
{
    std::vector<char> readFile(const std::string& filename);
    std::vector<unsigned int> compileToSPV(const vk::ShaderStageFlagBits shaderType,
                                           std::string const& glslShader);

    class ShaderModule
    {
    public:
        ShaderModule(const Context& context,
                     const std::string& shaderText,
                     vk::ShaderStageFlagBits shaderStage)
            : shaderStage(shaderStage)
        {
            std::vector<unsigned int> shaderSPV = compileToSPV(shaderStage, shaderText);
            vk::ShaderModuleCreateInfo createInfo{ {}, shaderSPV };
            shaderModule = context.getDevice().createShaderModuleUnique(createInfo);
        }

        ShaderModule(const ShaderModule&) = delete;
        ShaderModule(ShaderModule&&) = default;
        ShaderModule& operator=(const ShaderModule&) = delete;
        ShaderModule& operator=(ShaderModule&&) = default;

        vk::PipelineShaderStageCreateInfo getStageInfo() const
        {
            vk::PipelineShaderStageCreateInfo stageInfo;
            stageInfo.setStage(shaderStage);
            stageInfo.setModule(*shaderModule);
            stageInfo.setPName("main");
            return stageInfo;
        }

        vk::ShaderModule get() const { return *shaderModule; }

    private:
        vk::UniqueShaderModule shaderModule;
        vk::ShaderStageFlagBits shaderStage;
    };
}
