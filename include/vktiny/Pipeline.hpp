#include "Context.hpp"
#include <fstream>

namespace vkt
{
    std::vector<char> readFile(const std::string& filename)
    {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("failed to open file!: " + filename);
        }
        size_t fileSize = (size_t)file.tellg();
        std::vector<char> buffer(fileSize);
        file.seekg(0);
        file.read(buffer.data(), fileSize);
        file.close();
        return buffer;
    }

    class Pipeline
    {
        Pipeline() = default;
        Pipeline(const Pipeline&) = delete;
        Pipeline& operator = (const Pipeline&) = delete;

    protected:
        vk::raii::Pipeline pipeline = nullptr;
        vk::raii::PipelineLayout layout = nullptr;
    };

    class ComputePipeline : public Pipeline
    {
    public:
        void initialize(const Context& context,
                        const std::vector<vk::DescriptorSetLayoutBinding>& bindings,
                        vk::raii::ShaderModule shaderModule)
        {
            vk::raii::DescriptorSetLayout descSetLayout(context.getDevice(), { {}, bindings });
            layout = vk::raii::PipelineLayout(context.getDevice(), { {}, *descSetLayout });

            vk::PipelineShaderStageCreateInfo stageInfo;
            stageInfo.setStage(vk::ShaderStageFlagBits::eCompute);
            stageInfo.setModule(*shaderModule);
            stageInfo.setPName("main");

            vk::ComputePipelineCreateInfo pipelineInfo;
            pipelineInfo.setStage(stageInfo);
            pipelineInfo.setLayout(*layout);
            pipeline = vk::raii::Pipeline(context.getDevice(), nullptr, pipelineInfo);
        }
    };
}
