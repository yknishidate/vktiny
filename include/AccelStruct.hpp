#pragma once

#include "Buffer.hpp"
#include "Device.hpp"
#include "Mesh.hpp"

class AccelStruct
{
public:

    using vkBU = vk::BufferUsageFlagBits;
    using vkMP = vk::MemoryPropertyFlagBits;

    //void initialize(const Device& device,
    //                const PhysicalDevice& physicalDevice,
    //                vk::AccelerationStructureGeometryKHR geometry,
    //                vk::AccelerationStructureTypeKHR type,
    //                uint32_t primitiveCount)
    //{
    //    this->device = &device;
    //    this->physicalDevice = &physicalDevice;
    //    this->type = type;
    //    this->primitiveCount = primitiveCount;

    //    createBuffer(geometry);
    //    create();
    //}

    vk::WriteDescriptorSet createWrite()
    {
        asInfo = vk::WriteDescriptorSetAccelerationStructureKHR{ *accelStruct };
        vk::WriteDescriptorSet asWrite;
        asWrite.setDescriptorCount(1);
        asWrite.setPNext(&asInfo);
        return asWrite;
    }

protected:

    vk::DeviceSize getSize(vk::AccelerationStructureBuildGeometryInfoKHR geometryInfo,
                           uint32_t primitiveCount)
    {
        auto buildSizes = device->get().getAccelerationStructureBuildSizesKHR(
            vk::AccelerationStructureBuildTypeKHR::eDevice, geometryInfo, primitiveCount);
        return buildSizes.accelerationStructureSize;
    }

    void createBuffer(vk::DeviceSize size)
    {
        //geometryInfo.setType(type);
        //geometryInfo.setFlags(vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace);
        //geometryInfo.setGeometries(geometry);

        //auto buildSizes = device->get().getAccelerationStructureBuildSizesKHR(
        //    vk::AccelerationStructureBuildTypeKHR::eDevice, geometryInfo, primitiveCount);
        //this->size = buildSizes.accelerationStructureSize;

        buffer.initialize(*device, *physicalDevice, size,
                          vkBU::eAccelerationStructureStorageKHR | vkBU::eShaderDeviceAddress,
                          vkMP::eDeviceLocal);
    }

    void createAccelStruct(vk::DeviceSize size, vk::AccelerationStructureTypeKHR type)
    {
        vk::AccelerationStructureCreateInfoKHR createInfo;
        createInfo.setBuffer(buffer.get());
        createInfo.setSize(size);
        createInfo.setType(type);
        accelStruct = device->get().createAccelerationStructureKHRUnique(createInfo);
    }

    void build(vk::CommandBuffer commandBuffer,
               vk::AccelerationStructureBuildGeometryInfoKHR geometryInfo,
               vk::DeviceSize size,
               uint32_t primitiveCount)
    {
        Buffer scratchBuffer;
        scratchBuffer.initialize(*device, *physicalDevice, size,
                                 vkBU::eStorageBuffer | vkBU::eShaderDeviceAddress,
                                 vkMP::eDeviceLocal);
        geometryInfo.setScratchData(scratchBuffer.getDeviceAddress());
        geometryInfo.setDstAccelerationStructure(*accelStruct);

        vk::AccelerationStructureBuildRangeInfoKHR rangeInfo{ primitiveCount , 0, 0, 0 };

        commandBuffer.buildAccelerationStructuresKHR(geometryInfo, &rangeInfo);
    }


    const Device* device;
    const PhysicalDevice* physicalDevice;

    vk::UniqueAccelerationStructureKHR accelStruct;
    Buffer buffer;

    //vk::AccelerationStructureTypeKHR type;
    //uint32_t primitiveCount;
    //vk::DeviceSize size;
    //vk::AccelerationStructureBuildGeometryInfoKHR geometryInfo;
    //uint64_t deviceAddress;

    vk::WriteDescriptorSetAccelerationStructureKHR asInfo;
};

class BottomLevelAccelStruct : public AccelStruct
{
public:
    void initialize(const Device& device,
                    const PhysicalDevice& physicalDevice,
                    const std::vector<Vertex>& vertices, const Buffer& vertexBuffer,
                    const std::vector<Index>& indices, const Buffer& indexBuffer)
    {
        this->device = &device;
        this->physicalDevice = &physicalDevice;

        vk::AccelerationStructureGeometryTrianglesDataKHR triangleData;
        triangleData.setVertexFormat(vk::Format::eR32G32B32Sfloat);
        triangleData.setVertexData(vertexBuffer.getDeviceAddress());
        triangleData.setVertexStride(sizeof(Vertex));
        triangleData.setMaxVertex(vertices.size());
        triangleData.setIndexType(vk::IndexType::eUint32);
        triangleData.setIndexData(indexBuffer.getDeviceAddress());

        vk::AccelerationStructureGeometryKHR geometry;
        geometry.setGeometryType(vk::GeometryTypeKHR::eTriangles);
        geometry.setGeometry({ triangleData });
        geometry.setFlags(vk::GeometryFlagBitsKHR::eOpaque);

        uint32_t primitiveCount = indices.size() / 3;

        auto type = vk::AccelerationStructureTypeKHR::eBottomLevel;

        vk::AccelerationStructureBuildGeometryInfoKHR geometryInfo;
        geometryInfo.setType(type);
        geometryInfo.setFlags(vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace);
        geometryInfo.setGeometries(geometry);

        // Create buffer and accel struct
        vk::DeviceSize size = getSize(geometryInfo, primitiveCount);
        createBuffer(size);
        createAccelStruct(size, type);

        // Build
        auto cmdBuf = device.beginGraphicsCommand();
        build(*cmdBuf, geometryInfo, size, primitiveCount);
        device.endGraphicsCommand(*cmdBuf);
    }
};

class TopLevelAccelStruct : public AccelStruct
{
public:
    void initialize(const Device& device,
                    const PhysicalDevice& physicalDevice,
                    vk::AccelerationStructureGeometryKHR geometry,
                    vk::AccelerationStructureTypeKHR type,
                    uint32_t primitiveCount)
    {

    }
};
