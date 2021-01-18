#pragma once
#include "vkbase.hpp"



//#define TINYGLTF_NO_STB_IMAGE_WRITE
//#ifdef VK_USE_PLATFORM_ANDROID_KHR
//#define TINYGLTF_ANDROID_LOAD_FROM_ASSETS
//#endif
//#include <tiny_gltf.h>

namespace vkr
{
    struct AccelerationStructureInstance
    {
        uint32_t modelIndex;
        glm::mat4 transformMatrix;
        uint32_t textureOffset;
    };

    namespace
    {
        vk::TransformMatrixKHR toVkMatrix(const glm::mat4 transformMatrix)
        {
            const glm::mat4 transposedMatrix = glm::transpose(transformMatrix);
            std::array<std::array<float, 4>, 3> data;
            std::memcpy(&data, &transposedMatrix, sizeof(vk::TransformMatrixKHR));
            return vk::TransformMatrixKHR(data);
        }
    }

    class AccelerationStructure
    {
    public:
        AccelerationStructure(const Device& device) : device(device) {}
        AccelerationStructure(const AccelerationStructure&) = delete;
        AccelerationStructure& operator = (const AccelerationStructure&) = delete;
        AccelerationStructure& operator = (AccelerationStructure&&) = delete;

        AccelerationStructure(AccelerationStructure&& other) noexcept;

        const Device& getDevice() const { return device; }
        const uint64_t getDeviceAddress() const { return deviceAddress; }

        vk::AccelerationStructureKHR& getHandle() { return *accelerationStructure; }

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
    };


    class TopLevelAccelerationStructure final : public AccelerationStructure
    {
    public:
        // TODO: vector input
        //TopLevelAccelerationStructure(const Device& device, std::vector<AccelerationStructureInstance>& instances);
        TopLevelAccelerationStructure(const Device& device, BottomLevelAccelerationStructure& blas, AccelerationStructureInstance& instance);

        TopLevelAccelerationStructure(const BottomLevelAccelerationStructure&) = delete;
        TopLevelAccelerationStructure& operator = (const BottomLevelAccelerationStructure&) = delete;
        TopLevelAccelerationStructure& operator = (BottomLevelAccelerationStructure&&) = delete;

        TopLevelAccelerationStructure(BottomLevelAccelerationStructure&& other) noexcept;

    };


    ////////////////////
    // implementation //
    ////////////////////


    void AccelerationStructure::build(vk::AccelerationStructureGeometryKHR& geometry,
        const vk::AccelerationStructureTypeKHR& asType, uint32_t primitiveCount)
    {
        vk::AccelerationStructureBuildGeometryInfoKHR buildGeometryInfo{};
        buildGeometryInfo.setType(asType);
        buildGeometryInfo.setFlags(vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace);
        buildGeometryInfo.setGeometries(geometry);

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
        accelerationBuildGeometryInfo.setType(asType);
        accelerationBuildGeometryInfo.setFlags(vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace);
        accelerationBuildGeometryInfo.setDstAccelerationStructure(*accelerationStructure);
        accelerationBuildGeometryInfo.setGeometries(geometry);
        accelerationBuildGeometryInfo.setScratchData(scratchBuffer.getDeviceAddress());

        vk::AccelerationStructureBuildRangeInfoKHR accelerationStructureBuildRangeInfo{};
        accelerationStructureBuildRangeInfo
            .setPrimitiveCount(primitiveCount) // TODO: primitiveCount ?
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
        // TODO: onDevice
        buffer = std::make_unique<Buffer>(device, size, usage, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
    }

    BottomLevelAccelerationStructure::BottomLevelAccelerationStructure(const Device& device, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices)
        : AccelerationStructure(device)
    {
        auto vertexBuffer = device.createVertexBuffer(vertices, false);
        auto indexBuffer = device.createIndexBuffer(indices, false);

        vk::AccelerationStructureGeometryTrianglesDataKHR triangleData{};
        triangleData.setVertexFormat(vk::Format::eR32G32B32Sfloat);
        triangleData.setVertexData(vertexBuffer->getDeviceAddress());
        triangleData.setVertexStride(sizeof(Vertex));
        triangleData.setMaxVertex(vertices.size());
        triangleData.setIndexType(vk::IndexType::eUint32);
        triangleData.setIndexData(indexBuffer->getDeviceAddress());

        vk::AccelerationStructureGeometryKHR geometry{};
        geometry.setGeometryType(vk::GeometryTypeKHR::eTriangles);
        geometry.setGeometry({ triangleData });
        geometry.setFlags(vk::GeometryFlagBitsKHR::eOpaque);

        uint32_t triangleCount = indices.size() / 3;
        build(geometry, vk::AccelerationStructureTypeKHR::eBottomLevel, triangleCount);

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

    // TODO: これは実験用にしておく
    TopLevelAccelerationStructure::TopLevelAccelerationStructure(const Device& device, BottomLevelAccelerationStructure& blas, AccelerationStructureInstance& instance)
        : AccelerationStructure(device)
    {
        vk::AccelerationStructureInstanceKHR asInstance{};
        asInstance.setTransform(toVkMatrix(instance.transformMatrix));
        asInstance.setInstanceCustomIndex(instance.modelIndex);
        asInstance.setMask(0xFF);
        asInstance.setInstanceShaderBindingTableRecordOffset(0);
        asInstance.setFlags(vk::GeometryInstanceFlagBitsKHR::eTriangleFacingCullDisable);
        asInstance.setAccelerationStructureReference(blas.getDeviceAddress());

        vk::DeviceSize size{ sizeof(VkAccelerationStructureInstanceKHR) };
        Buffer instancesBuffer{ device, size,
            vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR | vk::BufferUsageFlagBits::eShaderDeviceAddress,
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, &asInstance };

        vk::AccelerationStructureGeometryInstancesDataKHR instancesData{};
        instancesData.setArrayOfPointers(false);
        instancesData.setData(instancesBuffer.getDeviceAddress());

        vk::AccelerationStructureGeometryKHR geometry{};
        geometry.setGeometryType(vk::GeometryTypeKHR::eInstances);
        geometry.setGeometry({ instancesData });
        geometry.setFlags(vk::GeometryFlagBitsKHR::eOpaque);

        uint32_t instanceCount = 1;
        build(geometry, vk::AccelerationStructureTypeKHR::eTopLevel, instanceCount);
    }


} // vkr
