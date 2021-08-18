#pragma once
#include "Context.hpp"

class Buffer
{
public:
    Buffer() = default;
    Buffer(const Buffer&) = delete;
    Buffer(Buffer&&) = default;

    vk::Buffer get() const { return *buffer; }

    void initialize(const Context& context,
                    vk::DeviceSize size, vk::BufferUsageFlags usage,
                    vk::MemoryPropertyFlags properties,
                    void* data = nullptr);

    void copy(void* data);

    vk::WriteDescriptorSet createWrite();
    uint64_t getDeviceAddress() const { return deviceAddress; }

private:
    void create(vk::DeviceSize size, vk::BufferUsageFlags usage);
    void allocate(vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties);

    const Context* context;

    vk::UniqueBuffer buffer;
    vk::UniqueDeviceMemory memory;
    vk::DeviceSize size;
    void* mapped = nullptr;

    uint64_t deviceAddress;
    vk::DescriptorBufferInfo bufferInfo;
};
