#pragma once
#include "vktiny/Context.hpp"

namespace vkt
{
    class Buffer
    {
    public:
        Buffer() = default;
        Buffer(const Buffer &) = delete;
        Buffer(Buffer &&) = default;
        Buffer &operator=(const Buffer &) = delete;
        Buffer &operator=(Buffer &&) = default;

        vk::Buffer get() const { return *buffer; }

        void initialize(const Context &context,
                        vk::DeviceSize size, vk::BufferUsageFlags usage,
                        vk::MemoryPropertyFlags properties,
                        void *data = nullptr);

        void copy(void *data);

        void copyOnHost(void *data)
        {
            if (!mapped)
            {
                mapped = context->getVkDevice().mapMemory(*memory, 0, size);
            }
            memcpy(mapped, data, static_cast<size_t>(size));
        }

        void copyOnDevice(void *data)
        {
            //Buffer stagingBuffer;
            //stagingBuffer.initialize(*context, size, vk::BufferUsageFlagBits::eTransferSrc, // TODO: add usage?
            //                         vkMP::eHostVisible | vkMP::eHostCoherent, data);
            //copyFrom(stagingBuffer);
        }

        vk::WriteDescriptorSet createWrite(); // TODO: remove this
        uint64_t getDeviceAddress() const { return deviceAddress; }

    private:
        void create(vk::DeviceSize size, vk::BufferUsageFlags usage);
        void allocate(vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties);

        const Context *context;

        vk::UniqueBuffer buffer;
        vk::UniqueDeviceMemory memory;
        vk::DeviceSize size;
        void *mapped = nullptr;

        uint64_t deviceAddress;
        vk::DescriptorBufferInfo bufferInfo;
    };
}
