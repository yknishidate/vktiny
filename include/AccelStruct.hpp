#pragma once

#include "Buffer.hpp"
#include "PhysicalDevice.hpp"

struct AccelStruct
{
    vk::UniqueAccelerationStructureKHR accelStruct;
    Buffer buffer;

    //vk::Device device;
    //vk::PhysicalDevice physicalDevice;
    //vk::AccelerationStructureTypeKHR type;
    //uint32_t primitiveCount;
    vk::DeviceSize size;
    vk::AccelerationStructureBuildGeometryInfoKHR geometryInfo;
    uint64_t deviceAddress;
    vk::WriteDescriptorSetAccelerationStructureKHR asInfo;

    void initialize(const Device& device,
                    const PhysicalDevice& physicalDevice)
    {

    }

    //void createBuffer(vk::Device device,
    //                  vk::PhysicalDevice physicalDevice,
    //                  vk::AccelerationStructureGeometryKHR geometry,
    //                  vk::AccelerationStructureTypeKHR type,
    //                  uint32_t primitiveCount)
    //{
    //    //this->device = device;
    //    //this->physicalDevice = physicalDevice;
    //    //this->type = type;
    //    //this->primitiveCount = primitiveCount;

    //    geometryInfo.setType(type);
    //    geometryInfo.setFlags(vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace);
    //    geometryInfo.setGeometries(geometry);

    //    auto buildSizesInfo = device.getAccelerationStructureBuildSizesKHR(vk::AccelerationStructureBuildTypeKHR::eDevice, geometryInfo, primitiveCount);
    //    size = buildSizesInfo.accelerationStructureSize;
    //    buffer.create(device, size, vkBU::eAccelerationStructureStorageKHR | vkBU::eShaderDeviceAddress);
    //    buffer.allocate(physicalDevice, vkMP::eDeviceLocal);
    //}

    //void create()
    //{
    //    vk::AccelerationStructureCreateInfoKHR createInfo;
    //    createInfo.setBuffer(*buffer.buffer);
    //    createInfo.setSize(size);
    //    createInfo.setType(type);
    //    handle = device.createAccelerationStructureKHRUnique(createInfo);
    //}

    //void build(vk::CommandBuffer commandBuffer)
    //{
    //    Buffer scratchBuffer;
    //    scratchBuffer.create(device, size, vkBU::eStorageBuffer | vkBU::eShaderDeviceAddress);
    //    scratchBuffer.allocate(physicalDevice, vkMP::eDeviceLocal);
    //    geometryInfo.setScratchData(scratchBuffer.deviceAddress);
    //    geometryInfo.setDstAccelerationStructure(*handle);

    //    vk::AccelerationStructureBuildRangeInfoKHR rangeInfo{ primitiveCount , 0, 0, 0 };
    //    commandBuffer.buildAccelerationStructuresKHR(geometryInfo, &rangeInfo);
    //}

    //vk::WriteDescriptorSet createDescWrite(vk::DescriptorSet& descSet, uint32_t binding)
    //{
    //    asInfo = vk::WriteDescriptorSetAccelerationStructureKHR{ *handle };
    //    vk::WriteDescriptorSet asWrite;
    //    asWrite.setDstSet(descSet);
    //    asWrite.setDescriptorType(vkDT::eAccelerationStructureKHR);
    //    asWrite.setDescriptorCount(1);
    //    asWrite.setDstBinding(binding);
    //    asWrite.setPNext(&asInfo);
    //    return asWrite;
    //}
};
