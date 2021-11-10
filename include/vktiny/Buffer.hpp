#pragma once
#include "Context.hpp"

namespace vkt
{
    class Buffer
    {
    public:
        Buffer() = default;
        Buffer(const Buffer&) = delete;
        Buffer& operator = (const Buffer&) = delete;

        void initialize(const Context& context,
                        vk::DeviceSize size,
                        vk::BufferUsageFlags usage,
                        vk::MemoryPropertyFlags propertyFlags)
        {
            buffer = vk::raii::Buffer(context.getDevice(), { {}, size, usage });

            vk::MemoryRequirements requirements = buffer.getMemoryRequirements();
            uint32_t memoryTypeIndex = context.findMemoryType(requirements, propertyFlags);
            vk::MemoryAllocateInfo allocInfo{ requirements.size, memoryTypeIndex };

            if (usage & vk::BufferUsageFlagBits::eShaderDeviceAddress) {
                vk::MemoryAllocateFlagsInfo flagsInfo{ vk::MemoryAllocateFlagBits::eDeviceAddress };
                allocInfo.pNext = &flagsInfo;
            }
            deviceMemory = vk::raii::DeviceMemory(context.getDevice(), allocInfo);
            buffer.bindMemory(*deviceMemory, 0);
        }

        vk::DeviceAddress getDeviceAddress()
        {
            vk::BufferDeviceAddressInfoKHR addressInfo{ *buffer };
            return buffer.getDevice().getBufferAddress(&addressInfo);
        }

    protected:
        vk::raii::Buffer buffer = nullptr;
        vk::raii::DeviceMemory deviceMemory = nullptr;
    };

    class HostBuffer : public Buffer
    {
    public:
        void initialize(const Context& context,
                        vk::DeviceSize size,
                        vk::BufferUsageFlags usage)
        {
            using vkMP = vk::MemoryPropertyFlagBits;
            Buffer::initialize(context, size, usage, vkMP::eHostVisible | vkMP::eHostCoherent);
        }

        template <typename T>
        void upload(const T& data)
        {
            if (!mapped) {
                mapped = deviceMemory.mapMemory(0, sizeof(T));
            }
            memcpy(mapped, &data, sizeof(T));
        }

    private:
        void* mapped;
    };
}
