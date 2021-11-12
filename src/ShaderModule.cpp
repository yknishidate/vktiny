#include "vktiny/Context.hpp"
#include "vktiny/ShaderModule.hpp"
#include <fstream>
#include <SPIRV/GlslangToSpv.h>
#include <StandAlone/ResourceLimits.h>

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

    EShLanguage translateShaderStage(vk::ShaderStageFlagBits stage)
    {
        switch (stage) {
            case vk::ShaderStageFlagBits::eVertex:
                return EShLangVertex;
            case vk::ShaderStageFlagBits::eTessellationControl:
                return EShLangTessControl;
            case vk::ShaderStageFlagBits::eTessellationEvaluation:
                return EShLangTessEvaluation;
            case vk::ShaderStageFlagBits::eGeometry:
                return EShLangGeometry;
            case vk::ShaderStageFlagBits::eFragment:
                return EShLangFragment;
            case vk::ShaderStageFlagBits::eCompute:
                return EShLangCompute;
            case vk::ShaderStageFlagBits::eRaygenNV:
                return EShLangRayGenNV;
            case vk::ShaderStageFlagBits::eAnyHitNV:
                return EShLangAnyHitNV;
            case vk::ShaderStageFlagBits::eClosestHitNV:
                return EShLangClosestHitNV;
            case vk::ShaderStageFlagBits::eMissNV:
                return EShLangMissNV;
            case vk::ShaderStageFlagBits::eIntersectionNV:
                return EShLangIntersectNV;
            case vk::ShaderStageFlagBits::eCallableNV:
                return EShLangCallableNV;
            case vk::ShaderStageFlagBits::eTaskNV:
                return EShLangTaskNV;
            case vk::ShaderStageFlagBits::eMeshNV:
                return EShLangMeshNV;
            default:
                assert(false && "Unknown shader stage");
                return EShLangVertex;
        }
    }

    std::vector<unsigned int> compileToSPV(const vk::ShaderStageFlagBits shaderType,
                                           std::string const& glslShader)
    {
        glslang::InitializeProcess();

        EShLanguage stage = translateShaderStage(shaderType);

        const char* shaderStrings[1];
        shaderStrings[0] = glslShader.data();

        glslang::TShader shader(stage);
        shader.setStrings(shaderStrings, 1);

        // Enable SPIR-V and Vulkan rules when parsing GLSL
        EShMessages messages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules);

        if (!shader.parse(&glslang::DefaultTBuiltInResource, 100, false, messages)) {
            throw std::runtime_error(shader.getInfoLog());
        }

        glslang::TProgram program;
        program.addShader(&shader);

        //
        // Program-level processing...
        //

        if (!program.link(messages)) {
            throw std::runtime_error(shader.getInfoLog());
        }

        std::vector<unsigned int> spvShader;
        glslang::GlslangToSpv(*program.getIntermediate(stage), spvShader);
        glslang::FinalizeProcess();

        return spvShader;
    }
    ShaderModule::ShaderModule(const Context& context, const std::string& shaderText, vk::ShaderStageFlagBits shaderStage)
        : shaderStage(shaderStage)
    {
        std::vector<unsigned int> shaderSPV = compileToSPV(shaderStage, shaderText);
        vk::ShaderModuleCreateInfo createInfo{ {}, shaderSPV };
        shaderModule = context.getDevice().createShaderModuleUnique(createInfo);
    }

    vk::PipelineShaderStageCreateInfo ShaderModule::getStageInfo() const
    {
        vk::PipelineShaderStageCreateInfo stageInfo;
        stageInfo.setStage(shaderStage);
        stageInfo.setModule(*shaderModule);
        stageInfo.setPName("main");
        return stageInfo;
    }
}
