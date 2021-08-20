#pragma once

#include "vktiny/Vulkan/Buffer.hpp"
#include "vktiny/Vulkan/Context.hpp"
#include "vktiny/Scene/Mesh.hpp"

namespace vkt
{
    class AccelStruct
    {
    public:
        AccelStruct() = default;
        AccelStruct(const AccelStruct&) = delete;
        AccelStruct(AccelStruct&&) = default;
        AccelStruct& operator = (const AccelStruct&) = delete;
        AccelStruct& operator = (AccelStruct&&) = default;

        using vkBU = vk::BufferUsageFlagBits;
        using vkMP = vk::MemoryPropertyFlagBits;

        const auto& get() const { return accelStruct; }
        auto getDeviceAddress() const { return deviceAddress; }

        vk::WriteDescriptorSet createWrite()
        {
            asInfo = { *accelStruct };
            vk::WriteDescriptorSet asWrite;
            asWrite.setDescriptorCount(1);
            asWrite.setPNext(&asInfo);
            return asWrite;
        }

        void initialize(const Context& context)
        {
            this->context = &context;
        }

        void build(vk::AccelerationStructureGeometryKHR& geometry,
                   const vk::AccelerationStructureTypeKHR& type,
                   uint32_t primitiveCount)
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

    protected:
        void createBuffer()
        {
            buffer.initialize(*context, size,
                              vkBU::eAccelerationStructureStorageKHR | vkBU::eShaderDeviceAddress,
                              vkMP::eDeviceLocal);
        }
        void createAccelStruct()
        {
            vk::AccelerationStructureCreateInfoKHR createInfo;
            createInfo.setBuffer(buffer.get());
            createInfo.setSize(size);
            createInfo.setType(type);
            accelStruct = context->getVkDevice().createAccelerationStructureKHRUnique(createInfo);
        }
        void createScratchBuffer()
        {
            scratchBuffer.initialize(*context,
                                     scratchSize,
                                     vkBU::eStorageBuffer | vkBU::eShaderDeviceAddress,
                                     vkMP::eDeviceLocal);
        }
        void getSizes()
        {
            auto buildSizesInfo = context->getVkDevice().getAccelerationStructureBuildSizesKHR(
                vk::AccelerationStructureBuildTypeKHR::eDevice, buildGeometryInfo, primitiveCount);
            size = buildSizesInfo.accelerationStructureSize;
            scratchSize = buildSizesInfo.buildScratchSize;
        }

        const Context* context;

        vk::UniqueAccelerationStructureKHR accelStruct;
        Buffer buffer;
        Buffer scratchBuffer;

        vk::AccelerationStructureBuildGeometryInfoKHR buildGeometryInfo;
        vk::AccelerationStructureTypeKHR type;
        vk::DeviceSize size;
        vk::DeviceSize scratchSize;
        uint64_t deviceAddress;
        uint32_t primitiveCount;

        vk::WriteDescriptorSetAccelerationStructureKHR asInfo;
    };

    class BottomLevelAccelStruct : public AccelStruct
    {
    public:
        BottomLevelAccelStruct() = default;
        BottomLevelAccelStruct(const BottomLevelAccelStruct&) = delete;
        BottomLevelAccelStruct(BottomLevelAccelStruct&&) = default;
        BottomLevelAccelStruct& operator = (const BottomLevelAccelStruct&) = delete;
        BottomLevelAccelStruct& operator = (BottomLevelAccelStruct&&) = default;

        void initialize(const Context& context,
                        const std::vector<Vertex>& vertices, const Buffer& vertexBuffer,
                        const std::vector<Index>& indices, const Buffer& indexBuffer);

        void initialize(const Context& context,
                        const Mesh& mesh);
    };

    class TopLevelAccelStruct : public AccelStruct
    {
    public:
        TopLevelAccelStruct() = default;
        TopLevelAccelStruct(const TopLevelAccelStruct&) = delete;
        TopLevelAccelStruct(TopLevelAccelStruct&&) = default;
        TopLevelAccelStruct& operator = (const TopLevelAccelStruct&) = delete;
        TopLevelAccelStruct& operator = (TopLevelAccelStruct&&) = default;

        void initialize(const Context& context,
                        const BottomLevelAccelStruct& bottomLevelAS,
                        const glm::mat4& transform = glm::mat4{ 1.0 });
        void initialize(const Context& context,
                        const std::vector<BottomLevelAccelStruct>& bottomLevelASs);

    private:
        std::vector<vk::AccelerationStructureInstanceKHR> instances;
        Buffer instancesBuffer;
        vk::DeviceSize instancesSize;
        vk::AccelerationStructureGeometryInstancesDataKHR instancesData;
        vk::AccelerationStructureGeometryKHR geometry;
    };
}
