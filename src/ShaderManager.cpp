#include "vktiny/ShaderManager.hpp"
#include "vktiny/Pipeline.hpp"

namespace vkt
{
    //void ShaderManager::initialize(const Context& context)
    //{
    //    this->context = &context;
    //}

    //void ShaderManager::addShader(const std::string filepath,
    //                              vk::ShaderStageFlagBits shaderStage)
    //{
    //    auto& shaderModule = addShaderModule(filepath);
    //    uint32_t stageIndex = addShaderStage(shaderStage, shaderModule);
    //}

    //void ShaderManager::addShaderFromText(const std::string& shaderText,
    //                                      vk::ShaderStageFlagBits shaderStage)
    //{
    //    std::vector<unsigned int> shaderSPV = compileToSPV(shaderStage, shaderText);

    //    vk::ShaderModuleCreateInfo createInfo{ {}, shaderSPV };
    //    modules.push_back(context->getDevice().createShaderModuleUnique(createInfo));
    //    uint32_t stageIndex = addShaderStage(shaderStage, *modules.back());
    //}

    //vk::ShaderModule& ShaderManager::addShaderModule(const std::string& filepath)
    //{
    //    const std::vector<char> code = readFile(filepath);
    //    vk::ShaderModuleCreateInfo createInfo;
    //    createInfo.setCodeSize(code.size());
    //    createInfo.setPCode(reinterpret_cast<const uint32_t*>(code.data()));
    //    modules.push_back(context->getDevice().createShaderModuleUnique(createInfo));
    //    return *modules.back();
    //}

    //uint32_t ShaderManager::addShaderStage(vk::ShaderStageFlagBits shaderStageFlag,
    //                                       const vk::ShaderModule& shaderModule)
    //{
    //    stages.push_back({ {}, shaderStageFlag, shaderModule, "main" });
    //    return static_cast<uint32_t>(stages.size() - 1);
    //}

    //void RayTracingShaderManager::addRaygenShader(const std::string filepath)
    //{
    //    raygenCount++;

    //    auto& shaderModule = addShaderModule(filepath);
    //    uint32_t stageIndex = addShaderStage(vkSS::eRaygenKHR, shaderModule);
    //    auto& shaderGroup = addShaderGroup(vkSGT::eGeneral);
    //    shaderGroup.generalShader = stageIndex;
    //}

    //void RayTracingShaderManager::addMissShader(const std::string filepath)
    //{
    //    missCount++;

    //    auto& shaderModule = addShaderModule(filepath);
    //    uint32_t stageIndex = addShaderStage(vkSS::eMissKHR, shaderModule);
    //    auto& shaderGroup = addShaderGroup(vkSGT::eGeneral);
    //    shaderGroup.generalShader = stageIndex;
    //}

    //void RayTracingShaderManager::addChitShader(const std::string filepath)
    //{
    //    hitCount++;

    //    auto& shaderModule = addShaderModule(filepath);
    //    uint32_t stageIndex = addShaderStage(vkSS::eClosestHitKHR, shaderModule);
    //    auto& shaderGroup = addShaderGroup(vkSGT::eTrianglesHitGroup);
    //    shaderGroup.generalShader = stageIndex;
    //}

    //void RayTracingShaderManager::addAhitShader(const std::string filepath)
    //{
    //    hitCount++;

    //    auto& shaderModule = addShaderModule(filepath);
    //    uint32_t stageIndex = addShaderStage(vkSS::eAnyHitKHR, shaderModule);
    //    auto& shaderGroup = addShaderGroup(vkSGT::eTrianglesHitGroup);
    //    shaderGroup.generalShader = stageIndex;
    //}

    //void RayTracingShaderManager::addChitAndAhitShader(const std::string chitFilepath,
    //                                                   const std::string ahitFilepath)
    //{
    //    hitCount += 2;

    //    auto& chitShaderModule = addShaderModule(chitFilepath);
    //    uint32_t chitIndex = addShaderStage(vkSS::eClosestHitKHR, chitShaderModule);

    //    auto& ahitShaderModule = addShaderModule(ahitFilepath);
    //    uint32_t ahitIndex = addShaderStage(vkSS::eAnyHitKHR, ahitShaderModule);

    //    auto& shaderGroup = addShaderGroup(vkSGT::eTrianglesHitGroup);
    //    shaderGroup.closestHitShader = chitIndex;
    //    shaderGroup.anyHitShader = ahitIndex;
    //}

    //void RayTracingShaderManager::initShaderBindingTable(const Pipeline& pipeline)
    //{
    //    // Get raytracing props
    //    const auto& props = context->getPhysicalDevice().getProperties2<vk::PhysicalDeviceProperties2, vk::PhysicalDeviceRayTracingPipelinePropertiesKHR>();
    //    const auto& rtProps = props.get<vk::PhysicalDeviceRayTracingPipelinePropertiesKHR>();

    //    // Calc SBT size
    //    const uint32_t handleSize = rtProps.shaderGroupHandleSize;
    //    const uint32_t handleSizeAligned = rtProps.shaderGroupHandleAlignment;
    //    const uint32_t groupCount = static_cast<uint32_t>(rtGroups.size());
    //    const uint32_t sbtSize = groupCount * handleSizeAligned;

    //    using vkBU = vk::BufferUsageFlagBits;
    //    using vkMP = vk::MemoryPropertyFlagBits;
    //    const vk::BufferUsageFlags usage =
    //        vkBU::eShaderBindingTableKHR | vkBU::eShaderDeviceAddress;
    //    const vk::MemoryPropertyFlags memProps = vkMP::eHostVisible | vkMP::eHostCoherent;

    //    // Get shader group handles
    //    // TODO: reduce args
    //    std::vector<uint8_t> shaderHandleStorage(sbtSize);
    //    auto result = context->getDevice().getRayTracingShaderGroupHandlesKHR(
    //        pipeline.get(), 0, groupCount, static_cast<size_t>(sbtSize),
    //        shaderHandleStorage.data());
    //    if (result != vk::Result::eSuccess) {
    //        throw std::runtime_error("failed to get ray tracing shader group handles.");
    //    }

    //    // Create SBT Buffers
    //    uint64_t raygenOffset = 0;
    //    uint64_t missOffset = raygenCount;
    //    uint64_t hitOffset = raygenCount + missCount;

    //    raygenSBT.initialize(*context, handleSize * raygenCount, usage, memProps);
    //    raygenSBT.copy(shaderHandleStorage.data() + raygenOffset * handleSizeAligned);

    //    missSBT.initialize(*context, handleSize * missCount, usage, memProps);
    //    missSBT.copy(shaderHandleStorage.data() + missOffset * handleSizeAligned);

    //    hitSBT.initialize(*context, handleSize * hitCount, usage, memProps);
    //    hitSBT.copy(shaderHandleStorage.data() + hitOffset * handleSizeAligned);

    //    raygenRegion.setDeviceAddress(raygenSBT.getDeviceAddress());
    //    raygenRegion.setStride(handleSizeAligned);
    //    raygenRegion.setSize(handleSizeAligned);

    //    missRegion.setDeviceAddress(missSBT.getDeviceAddress());
    //    missRegion.setStride(handleSizeAligned);
    //    missRegion.setSize(handleSizeAligned);

    //    hitRegion.setDeviceAddress(hitSBT.getDeviceAddress());
    //    hitRegion.setStride(handleSizeAligned);
    //    hitRegion.setSize(handleSizeAligned);
    //}

    //vk::RayTracingShaderGroupCreateInfoKHR& RayTracingShaderManager::addShaderGroup(
    //    vk::RayTracingShaderGroupTypeKHR type)
    //{
    //    vk::RayTracingShaderGroupCreateInfoKHR shaderGroup{ type };
    //    shaderGroup.setGeneralShader(VK_SHADER_UNUSED_KHR);
    //    shaderGroup.setClosestHitShader(VK_SHADER_UNUSED_KHR);
    //    shaderGroup.setAnyHitShader(VK_SHADER_UNUSED_KHR);
    //    shaderGroup.setIntersectionShader(VK_SHADER_UNUSED_KHR);
    //    rtGroups.push_back(shaderGroup);
    //    return rtGroups.back();
    //}
}