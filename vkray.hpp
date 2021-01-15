#pragma once
#include "vkbase.hpp"



//#define TINYGLTF_NO_STB_IMAGE_WRITE
//#ifdef VK_USE_PLATFORM_ANDROID_KHR
//#define TINYGLTF_ANDROID_LOAD_FROM_ASSETS
//#endif
//#include <tiny_gltf.h>

namespace vkr
{

    class AccelerationStructure
    {
    public:
        AccelerationStructure(const Device& device) : device(device) {}
        AccelerationStructure(const AccelerationStructure&) = delete;
        AccelerationStructure& operator = (const AccelerationStructure&) = delete;
        AccelerationStructure& operator = (AccelerationStructure&&) = delete;

        AccelerationStructure(AccelerationStructure&& other) noexcept;

        const Device& getDevice() const { return device; }

    protected:
        void build(vk::AccelerationStructureGeometryKHR& geometry, const vk::AccelerationStructureTypeKHR& asType, uint32_t primitiveCount);

        void createBuffer(vk::AccelerationStructureBuildSizesInfoKHR buildSizesInfo);

        Buffer createScratchBuffer(vk::DeviceSize size);

        const Device& device;

        vk::UniqueAccelerationStructureKHR accelerationStructure;

        std::unique_ptr<Buffer> buffer;

        uint64_t deviceAddress;
    };


    class BottomLevelAccelerationStructure final : public AccelerationStructure
    {
    public:
        BottomLevelAccelerationStructure(const Device& device, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);
        //BottomLevelAccelerationStructure(const Device& device, const Buffer& vertexBuffer, const Buffer& indexBuffer);

        BottomLevelAccelerationStructure(const BottomLevelAccelerationStructure&) = delete;
        BottomLevelAccelerationStructure& operator = (const BottomLevelAccelerationStructure&) = delete;
        BottomLevelAccelerationStructure& operator = (BottomLevelAccelerationStructure&&) = delete;

        BottomLevelAccelerationStructure(BottomLevelAccelerationStructure&& other) noexcept;

    private:

        //std::vector<VkGeometryNV> geometries_;
    };


    ////////////////////
    // implementation //
    ////////////////////


    void AccelerationStructure::build(vk::AccelerationStructureGeometryKHR& geometry,
        const vk::AccelerationStructureTypeKHR& asType, uint32_t primitiveCount)
    {
        vk::AccelerationStructureBuildGeometryInfoKHR buildGeometryInfo{};
        buildGeometryInfo
            .setType(asType)
            .setFlags(vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace)
            .setGeometries(geometry);

        auto buildSizesInfo = device.getHandle().getAccelerationStructureBuildSizesKHR(
            vk::AccelerationStructureBuildTypeKHR::eDevice, buildGeometryInfo, primitiveCount);

        createBuffer(buildSizesInfo);

        accelerationStructure = device.getHandle().createAccelerationStructureKHRUnique(
            vk::AccelerationStructureCreateInfoKHR{}
            .setBuffer(buffer->getHandle())
            .setSize(buildSizesInfo.accelerationStructureSize)
            .setType(asType)
        );

        Buffer scratchBuffer{ device, buildSizesInfo.buildScratchSize,
            vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress,
            vk::MemoryPropertyFlagBits::eDeviceLocal };

        vk::AccelerationStructureBuildGeometryInfoKHR accelerationBuildGeometryInfo{};
        accelerationBuildGeometryInfo
            .setType(asType)
            .setFlags(vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace)
            .setDstAccelerationStructure(*accelerationStructure)
            .setGeometries(geometry)
            .setScratchData(scratchBuffer.getDeviceAddress());

        vk::AccelerationStructureBuildRangeInfoKHR accelerationStructureBuildRangeInfo{};
        accelerationStructureBuildRangeInfo
            .setPrimitiveCount(1) // TODO: primitiveCount ?
            .setPrimitiveOffset(0)
            .setFirstVertex(0)
            .setTransformOffset(0);

        auto commandBuffer = device.createCommandBuffer(vk::CommandBufferLevel::ePrimary, true);
        commandBuffer->buildAccelerationStructuresKHR(accelerationBuildGeometryInfo, &accelerationStructureBuildRangeInfo);
        device.submitCommandBuffer(*commandBuffer);

        deviceAddress = device.getHandle().getAccelerationStructureAddressKHR({ *accelerationStructure });
    }

    void AccelerationStructure::createBuffer(vk::AccelerationStructureBuildSizesInfoKHR buildSizesInfo)
    {
        auto size = buildSizesInfo.accelerationStructureSize;
        auto usage = vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR | vk::BufferUsageFlagBits::eShaderDeviceAddress;
        buffer = std::make_unique<Buffer>(device, size, usage, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
    }

    BottomLevelAccelerationStructure::BottomLevelAccelerationStructure(const Device& device, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices)
        : AccelerationStructure(device)
    {
        auto vertexBuffer = device.createVertexBuffer(vertices, false);
        auto indexBuffer = device.createIndexBuffer(indices, false);

        vk::AccelerationStructureGeometryTrianglesDataKHR triangleData{};
        triangleData
            .setVertexFormat(vk::Format::eR32G32B32Sfloat)
            .setVertexData(vertexBuffer->getDeviceAddress())
            .setVertexStride(sizeof(Vertex))
            .setMaxVertex(vertices.size())
            .setIndexType(vk::IndexType::eUint32)
            .setIndexData(indexBuffer->getDeviceAddress());

        vk::AccelerationStructureGeometryKHR geometry{};
        geometry
            .setGeometryType(vk::GeometryTypeKHR::eTriangles)
            .setGeometry({ triangleData })
            .setFlags(vk::GeometryFlagBitsKHR::eOpaque);

        build(geometry, vk::AccelerationStructureTypeKHR::eBottomLevel, 1);

    }

    //BottomLevelAccelerationStructure::BottomLevelAccelerationStructure(const Device& device, Buffer& vertexBuffer, Buffer& indexBuffer)
    //    : AccelerationStructure(device)
    //{
    //    vk::AccelerationStructureGeometryTrianglesDataKHR triangleData{};
    //    triangleData
    //        .setVertexFormat(vk::Format::eR32G32B32Sfloat)
    //        .setVertexData(vertexBuffer.getDeviceAddress())
    //        .setVertexStride(sizeof(Vertex))
    //        .setMaxVertex(vertices.size())
    //        .setIndexType(vk::IndexType::eUint32)
    //        .setIndexData(indexBuffer.getDeviceAddress());
    //
    //    vk::AccelerationStructureGeometryKHR geometry{};
    //    geometry
    //        .setGeometryType(vk::GeometryTypeKHR::eTriangles)
    //        .setGeometry({ triangleData })
    //        .setFlags(vk::GeometryFlagBitsKHR::eOpaque);
    //
    //    build(geometry, vk::AccelerationStructureTypeKHR::eBottomLevel, 1);
    //}


} // vkr
