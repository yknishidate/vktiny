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

        const Buffer& getBuffer() const { return buffer; }
        auto getDeviceAddress() const { return deviceAddress; }

        vk::WriteDescriptorSet createWrite() // TODO: remove this
        {
            asInfo = vk::WriteDescriptorSetAccelerationStructureKHR{ *accelStruct };
            vk::WriteDescriptorSet asWrite;
            asWrite.setDescriptorCount(1);
            asWrite.setPNext(&asInfo);
            return asWrite;
        }

    protected:
        vk::DeviceSize getSize(vk::AccelerationStructureBuildGeometryInfoKHR geometryInfo,
                               uint32_t primitiveCount);

        void createBuffer(vk::DeviceSize size);

        void createAccelStruct(vk::DeviceSize size, vk::AccelerationStructureTypeKHR type);

        void build(vk::CommandBuffer commandBuffer,
                   vk::AccelerationStructureBuildGeometryInfoKHR geometryInfo,
                   vk::DeviceSize size,
                   uint32_t primitiveCount);

        const Context* context;

        vk::UniqueAccelerationStructureKHR accelStruct;
        Buffer buffer;

        vk::WriteDescriptorSetAccelerationStructureKHR asInfo;
        uint64_t deviceAddress;
        vk::DeviceSize scratchSize;
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
        //void initialize(const Context& context,
        //                const std::vector<BottomLevelAccelStruct>& bottomLevelASs,
        //                const std::vector<glm::mat4>& transforms);

    private:
        std::vector<vk::AccelerationStructureInstanceKHR> instances;
        Buffer instanceBuffer;
        vk::AccelerationStructureGeometryInstancesDataKHR instancesData;
        vk::AccelerationStructureGeometryKHR geometry;
        vk::AccelerationStructureBuildGeometryInfoKHR geometryInfo;
    };
}
