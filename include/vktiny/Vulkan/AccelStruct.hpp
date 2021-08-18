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
    };

    class BottomLevelAccelStruct : public AccelStruct
    {
    public:
        void initialize(const Context& context,
                        const std::vector<Vertex>& vertices, const Buffer& vertexBuffer,
                        const std::vector<Index>& indices, const Buffer& indexBuffer);

        void initialize(const Context& context,
                        const Mesh& mesh);
    };

    class TopLevelAccelStruct : public AccelStruct
    {
    public:
        void initialize(const Context& context,
                        const BottomLevelAccelStruct& bottomLevelAS,
                        const glm::mat4& transform = glm::mat4{ 1.0 });
    };
}
