#pragma once
#include "vktiny/Vulkan/Buffer.hpp"
#include "vktiny/Vulkan/Context.hpp"
#include <fstream>


namespace vkt
{
    class Pipeline;

    class ShaderManager
    {
    public:
        ShaderManager() = default;
        ShaderManager(const ShaderManager&) = delete;
        ShaderManager(ShaderManager&&) = default;
        ShaderManager& operator = (const ShaderManager&) = delete;
        ShaderManager& operator = (ShaderManager&&) = default;

        void initialize(const Context& context);

        const auto& getStages() const { return stages; }

    protected:
        vk::ShaderModule& addShaderModule(const std::string& filepath);

        uint32_t addShaderStage(vk::ShaderStageFlagBits shaderStageFlag,
                                const vk::ShaderModule& shaderModule);

        const Context* context;

        std::vector<vk::UniqueShaderModule> modules;
        std::vector<vk::PipelineShaderStageCreateInfo> stages;
    };

    class RayTracingShaderManager : public ShaderManager
    {
    public:
        using vkSS = vk::ShaderStageFlagBits;
        using vkSGT = vk::RayTracingShaderGroupTypeKHR;
        void addRaygenShader(const std::string filepath);
        void addMissShader(const std::string filepath);
        void addChitShader(const std::string filepath);
        void addAhitShader(const std::string filepath);
        void addChitAndAhitShader(const std::string chitFilepath, const std::string ahitFilepath);

        void initShaderBindingTable(const Pipeline& pipeline);

        const auto& getRtGroups() const { return rtGroups; }
        const auto& getRaygenRegion() const { return raygenRegion; }
        const auto& getMissRegion() const { return missRegion; }
        const auto& getHitRegion() const { return hitRegion; }

    private:
        vk::RayTracingShaderGroupCreateInfoKHR& addShaderGroup(
            vk::RayTracingShaderGroupTypeKHR type);

        std::vector<vk::RayTracingShaderGroupCreateInfoKHR> rtGroups;

        uint32_t raygenCount = 0;
        uint32_t missCount = 0;
        uint32_t hitCount = 0;

        Buffer raygenSBT;
        Buffer missSBT;
        Buffer hitSBT;

        vk::StridedDeviceAddressRegionKHR raygenRegion;
        vk::StridedDeviceAddressRegionKHR missRegion;
        vk::StridedDeviceAddressRegionKHR hitRegion;
    };

    class ComputeShaderManager : public ShaderManager
    {
    public:
        using vkSS = vk::ShaderStageFlagBits;
        void addComputeShader(const std::string filepath)
        {
            auto& shaderModule = addShaderModule(filepath);
            uint32_t stageIndex = addShaderStage(vkSS::eCompute, shaderModule);
        }
    };
}
