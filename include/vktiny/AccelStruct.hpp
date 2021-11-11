#pragma once

#include "vktiny/Buffer.hpp"
#include "vktiny/Context.hpp"

namespace vkt
{
    class AccelStruct
    {
    public:
        AccelStruct() = default;
        AccelStruct(const AccelStruct&) = delete;
        AccelStruct(AccelStruct&&) = default;
        AccelStruct& operator=(const AccelStruct&) = delete;
        AccelStruct& operator=(AccelStruct&&) = default;

        using vkBU = vk::BufferUsageFlagBits;
        using vkMP = vk::MemoryPropertyFlagBits;

        const auto& get() const { return accelStruct; }
        auto getDeviceAddress() const { return deviceAddress; }
        auto getAccelStructInfo() const { return asInfo; }

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
        BottomLevelAccelStruct& operator=(const BottomLevelAccelStruct&) = delete;
        BottomLevelAccelStruct& operator=(BottomLevelAccelStruct&&) = default;
    };

    class TopLevelAccelStruct : public AccelStruct
    {
    public:
        TopLevelAccelStruct() = default;
        TopLevelAccelStruct(const TopLevelAccelStruct&) = delete;
        TopLevelAccelStruct(TopLevelAccelStruct&&) = default;
        TopLevelAccelStruct& operator=(const TopLevelAccelStruct&) = delete;
        TopLevelAccelStruct& operator=(TopLevelAccelStruct&&) = default;

    private:
        std::vector<vk::AccelerationStructureInstanceKHR> instances;
        Buffer instancesBuffer;
        vk::DeviceSize instancesSize;
        vk::AccelerationStructureGeometryInstancesDataKHR instancesData;
        vk::AccelerationStructureGeometryKHR geometry;
    };
}
