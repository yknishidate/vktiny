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

        vk::WriteDescriptorSet createWrite();

        void initialize(const Context& context);

        void build(vk::AccelerationStructureGeometryKHR& geometry,
                   const vk::AccelerationStructureTypeKHR& type,
                   uint32_t primitiveCount);

    protected:
        void createBuffer();
        void createAccelStruct();
        void createScratchBuffer();
        void getSizes();

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
                        const std::vector<BottomLevelAccelStruct>& bottomLevelASs,
                        const glm::mat4& transform = glm::mat4{ 1.0 });

    private:
        std::vector<vk::AccelerationStructureInstanceKHR> instances;
        Buffer instancesBuffer;
        vk::DeviceSize instancesSize;
        vk::AccelerationStructureGeometryInstancesDataKHR instancesData;
        vk::AccelerationStructureGeometryKHR geometry;
    };
}
