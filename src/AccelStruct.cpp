
#include "vktiny/AccelStruct.hpp"

vk::WriteDescriptorSet vkt::AccelStruct::createWrite()
{
    asInfo = { *accelStruct };
    vk::WriteDescriptorSet asWrite;
    asWrite.setDescriptorCount(1);
    asWrite.setPNext(&asInfo);
    return asWrite;
}

void vkt::AccelStruct::initialize(const Context& context)
{
    this->context = &context;
}

void vkt::AccelStruct::build(vk::AccelerationStructureGeometryKHR& geometry, const vk::AccelerationStructureTypeKHR& type, uint32_t primitiveCount)
{
    this->type = type;
    this->primitiveCount = primitiveCount;

    buildGeometryInfo.setType(type);
    buildGeometryInfo.setFlags(vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace);
    buildGeometryInfo.setGeometries(geometry);

    getSizes();
    createBuffer();
    createAccelStruct();
    createScratchBuffer();

    buildGeometryInfo.setDstAccelerationStructure(*accelStruct);
    buildGeometryInfo.setScratchData(scratchBuffer.getDeviceAddress());

    vk::AccelerationStructureBuildRangeInfoKHR buildRangeInfo{};
    buildRangeInfo
        .setPrimitiveCount(primitiveCount)
        .setPrimitiveOffset(0)
        .setFirstVertex(0)
        .setTransformOffset(0);

    auto cmdBuf = context->getDevice().beginGraphicsCommand();
    cmdBuf->buildAccelerationStructuresKHR(buildGeometryInfo, &buildRangeInfo);
    context->getDevice().endGraphicsCommand(*cmdBuf);

    deviceAddress = context->getVkDevice().getAccelerationStructureAddressKHR({ *accelStruct });
}

void vkt::AccelStruct::createBuffer()
{
    buffer.initialize(*context, size,
                      vkBU::eAccelerationStructureStorageKHR | vkBU::eShaderDeviceAddress,
                      vkMP::eDeviceLocal);
}

void vkt::AccelStruct::createAccelStruct()
{
    vk::AccelerationStructureCreateInfoKHR createInfo;
    createInfo.setBuffer(buffer.get());
    createInfo.setSize(size);
    createInfo.setType(type);
    accelStruct = context->getVkDevice().createAccelerationStructureKHRUnique(createInfo);
}

void vkt::AccelStruct::createScratchBuffer()
{
    scratchBuffer.initialize(*context,
                             scratchSize,
                             vkBU::eStorageBuffer | vkBU::eShaderDeviceAddress,
                             vkMP::eDeviceLocal);
}

void vkt::AccelStruct::getSizes()
{
    auto buildSizesInfo = context->getVkDevice().getAccelerationStructureBuildSizesKHR(
        vk::AccelerationStructureBuildTypeKHR::eDevice, buildGeometryInfo, primitiveCount);
    size = buildSizesInfo.accelerationStructureSize;
    scratchSize = buildSizesInfo.buildScratchSize;
}
