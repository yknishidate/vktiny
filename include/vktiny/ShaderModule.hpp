#pragma once

#include <vulkan/vulkan.hpp>

namespace vkt
{
    class Context;

    std::vector<char> readFile(const std::string& filename);
    std::vector<unsigned int> compileToSPV(const vk::ShaderStageFlagBits shaderType,
                                           std::string const& glslShader);

    class ShaderModule
    {
    public:
        ShaderModule(const Context& context,
                     const std::string& shaderText,
                     vk::ShaderStageFlagBits shaderStage);

        ShaderModule(const ShaderModule&) = delete;
        ShaderModule(ShaderModule&&) = default;
        ShaderModule& operator=(const ShaderModule&) = delete;
        ShaderModule& operator=(ShaderModule&&) = default;

        vk::PipelineShaderStageCreateInfo getStageInfo() const;

        vk::ShaderModule get() const { return *shaderModule; }

    private:
        vk::UniqueShaderModule shaderModule;
        vk::ShaderStageFlagBits shaderStage;
    };
}
