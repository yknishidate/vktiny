#pragma once
#include "vkbase.hpp"

namespace vkr
{


    class AccelerationStructure
    {
    public:

        struct ScratchBuffer
        {
            uint64_t deviceAddress;
            vk::UniqueBuffer handle;
            vk::UniqueDeviceMemory memory;
        };

        AccelerationStructure(const AccelerationStructure&) = delete;
        AccelerationStructure& operator = (const AccelerationStructure&) = delete;
        AccelerationStructure& operator = (AccelerationStructure&&) = delete;

        AccelerationStructure(AccelerationStructure&& other) noexcept;
        virtual ~AccelerationStructure();

        const Device& getDevice() const { return device; }

    protected:

        void build(vk::AccelerationStructureGeometryKHR& geometry,
            const vk::AccelerationStructureTypeKHR& asType,
            uint32_t primitiveCount);

        void createAccelerationStructureBuffer(vk::AccelerationStructureBuildSizesInfoKHR buildSizesInfo);

    private:

        const Device& device;

        vk::UniqueAccelerationStructureKHR accelerationStructure;

        uint64_t deviceAddress;
        std::unique_ptr<Buffer> buffer;

    };

    void AccelerationStructure::build(vk::AccelerationStructureGeometryKHR& geometry,
        const vk::AccelerationStructureTypeKHR& asType,
        uint32_t primitiveCount)
    {
        // ASビルドに必要なサイズを取得する
        vk::AccelerationStructureBuildGeometryInfoKHR buildGeometryInfo{};
        buildGeometryInfo
            .setType(asType)
            .setFlags(vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace)
            .setGeometries(geometry);

        auto buildSizesInfo = device.getHandle().getAccelerationStructureBuildSizesKHR(
            vk::AccelerationStructureBuildTypeKHR::eDevice, buildGeometryInfo, primitiveCount);

        createAccelerationStructureBuffer(buildSizesInfo);
    }

    void AccelerationStructure::createAccelerationStructureBuffer(vk::AccelerationStructureBuildSizesInfoKHR buildSizesInfo)
    {
        auto size = buildSizesInfo.accelerationStructureSize;
        auto usage = vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR | vk::BufferUsageFlagBits::eShaderDeviceAddress;
        buffer = std::make_unique<Buffer>(device, size, usage);

        auto requirements = device.getHandle().getBufferMemoryRequirements(buffer->getHandle());
        vk::MemoryAllocateFlagsInfo allocateFlagsInfo{ vk::MemoryAllocateFlagBits::eDeviceAddress };
        buffer->allocateMemory(vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, allocateFlagsInfo);
    }
}
