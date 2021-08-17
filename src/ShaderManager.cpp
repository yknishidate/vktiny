#include "ShaderManager.hpp"
#include "Pipeline.hpp"

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

void RayTracingShaderManager::initShaderBindingTable(const Pipeline& pipeline)
{
    // Get raytracing props
    const auto& props = physicalDevice->get().getProperties2<
        vk::PhysicalDeviceProperties2,
        vk::PhysicalDeviceRayTracingPipelinePropertiesKHR>();
    const auto& rtProps = props.get<vk::PhysicalDeviceRayTracingPipelinePropertiesKHR>();

    // Calc SBT size
    const uint32_t handleSize = rtProps.shaderGroupHandleSize;
    const uint32_t handleSizeAligned = rtProps.shaderGroupHandleAlignment;
    const uint32_t groupCount = static_cast<uint32_t>(rtGroups.size());
    const uint32_t sbtSize = groupCount * handleSizeAligned;

    using vkBU = vk::BufferUsageFlagBits;
    using vkMP = vk::MemoryPropertyFlagBits;
    const vk::BufferUsageFlags usage = vkBU::eShaderBindingTableKHR | vkBU::eShaderDeviceAddress;
    const vk::MemoryPropertyFlags memProps = vkMP::eHostVisible | vkMP::eHostCoherent;

    // Get shader group handles
    // TODO: reduce args
    std::vector<uint8_t> shaderHandleStorage(sbtSize);
    auto result = device->get().getRayTracingShaderGroupHandlesKHR(
        pipeline.get(), 0, groupCount, static_cast<size_t>(sbtSize), shaderHandleStorage.data());
    if (result != vk::Result::eSuccess) {
        throw std::runtime_error("failed to get ray tracing shader group handles.");
    }

    // Create SBT Buffers
    uint64_t raygenOffset = 0;
    uint64_t missOffset = raygenCount;
    uint64_t hitOffset = raygenCount + missCount;

    raygenSBT.initialize(*device, *physicalDevice, handleSize * raygenCount, usage, memProps);
    raygenSBT.copy(shaderHandleStorage.data() + raygenOffset * handleSizeAligned);

    missSBT.initialize(*device, *physicalDevice, handleSize * missCount, usage, memProps);
    missSBT.copy(shaderHandleStorage.data() + missOffset * handleSizeAligned);

    hitSBT.initialize(*device, *physicalDevice, handleSize * hitCount, usage, memProps);
    hitSBT.copy(shaderHandleStorage.data() + hitOffset * handleSizeAligned);

    raygenRegion.setDeviceAddress(raygenSBT.getDeviceAddress());
    raygenRegion.setStride(handleSizeAligned);
    raygenRegion.setSize(handleSizeAligned);

    missRegion.setDeviceAddress(missSBT.getDeviceAddress());
    missRegion.setStride(handleSizeAligned);
    missRegion.setSize(handleSizeAligned);

    hitRegion.setDeviceAddress(hitSBT.getDeviceAddress());
    hitRegion.setStride(handleSizeAligned);
    hitRegion.setSize(handleSizeAligned);
}
