#pragma once
#include "Buffer.hpp"
#include "Context.hpp"
#include <fstream>

class Pipeline;

std::vector<char> readFile(const std::string& filename);

class ShaderManager
{
public:
    void initialize(const Context& context)
    {
        this->device = &context.getDevice();
        this->physicalDevice = &context.getPhysicalDevice();
    }

    const auto& getStages() const { return stages; }

protected:
    vk::ShaderModule& addShaderModule(const std::string& filepath)
    {
        const std::vector<char> code = readFile(filepath);
        vk::ShaderModuleCreateInfo createInfo;
        createInfo.setCodeSize(code.size());
        createInfo.setPCode(reinterpret_cast<const uint32_t*>(code.data()));
        modules.push_back(device->get().createShaderModuleUnique(createInfo));
        return *modules.back();
    }

    uint32_t addShaderStage(vk::ShaderStageFlagBits shaderStageFlag,
                            const vk::ShaderModule& shaderModule)
    {
        stages.push_back({ {}, shaderStageFlag, shaderModule, "main" });
        return static_cast<uint32_t>(stages.size() - 1);
    }

    const Device* device;
    const PhysicalDevice* physicalDevice;

    std::vector<vk::UniqueShaderModule> modules;
    std::vector<vk::PipelineShaderStageCreateInfo> stages;
};

class RayTracingShaderManager : public ShaderManager
{
public:
    using vkSS = vk::ShaderStageFlagBits;
    using vkSGT = vk::RayTracingShaderGroupTypeKHR;
    void addRaygenShader(const std::string filepath)
    {
        raygenCount++;

        auto& shaderModule = addShaderModule(filepath);
        uint32_t stageIndex = addShaderStage(vkSS::eRaygenKHR, shaderModule);
        auto& shaderGroup = addShaderGroup(vkSGT::eGeneral);
        shaderGroup.generalShader = stageIndex;
    }

    void addMissShader(const std::string filepath)
    {
        missCount++;

        auto& shaderModule = addShaderModule(filepath);
        uint32_t stageIndex = addShaderStage(vkSS::eMissKHR, shaderModule);
        auto& shaderGroup = addShaderGroup(vkSGT::eGeneral);
        shaderGroup.generalShader = stageIndex;
    }

    void addChitShader(const std::string filepath)
    {
        hitCount++;

        auto& shaderModule = addShaderModule(filepath);
        uint32_t stageIndex = addShaderStage(vkSS::eClosestHitKHR, shaderModule);
        auto& shaderGroup = addShaderGroup(vkSGT::eTrianglesHitGroup);
        shaderGroup.generalShader = stageIndex;
    }

    void addAhitShader(const std::string filepath)
    {
        hitCount++;

        auto& shaderModule = addShaderModule(filepath);
        uint32_t stageIndex = addShaderStage(vkSS::eAnyHitKHR, shaderModule);
        auto& shaderGroup = addShaderGroup(vkSGT::eTrianglesHitGroup);
        shaderGroup.generalShader = stageIndex;
    }

    void addChitAndAhitShader(const std::string chitFilepath, const std::string ahitFilepath)
    {
        hitCount += 2;

        auto& chitShaderModule = addShaderModule(chitFilepath);
        uint32_t chitIndex = addShaderStage(vkSS::eClosestHitKHR, chitShaderModule);

        auto& ahitShaderModule = addShaderModule(ahitFilepath);
        uint32_t ahitIndex = addShaderStage(vkSS::eAnyHitKHR, ahitShaderModule);

        auto& shaderGroup = addShaderGroup(vkSGT::eTrianglesHitGroup);
        shaderGroup.closestHitShader = chitIndex;
        shaderGroup.anyHitShader = ahitIndex;
    }

    void initShaderBindingTable(const Pipeline& pipeline);

    const auto& getRtGroups() const { return rtGroups; }
    const auto& getRaygenRegion() const { return raygenRegion; }
    const auto& getMissRegion() const { return missRegion; }
    const auto& getHitRegion() const { return hitRegion; }

private:
    vk::RayTracingShaderGroupCreateInfoKHR& addShaderGroup(vk::RayTracingShaderGroupTypeKHR type)
    {
        vk::RayTracingShaderGroupCreateInfoKHR shaderGroup{ type };
        shaderGroup.setGeneralShader(VK_SHADER_UNUSED_KHR);
        shaderGroup.setClosestHitShader(VK_SHADER_UNUSED_KHR);
        shaderGroup.setAnyHitShader(VK_SHADER_UNUSED_KHR);
        shaderGroup.setIntersectionShader(VK_SHADER_UNUSED_KHR);
        rtGroups.push_back(shaderGroup);
        return rtGroups.back();
    }

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
