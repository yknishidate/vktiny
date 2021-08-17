#pragma once
#include "Device.hpp"

class Buffer
{
public:
    Buffer() = default;
    Buffer(const Buffer&) = delete;
    Buffer(Buffer&&) = default;

    void initialize(const Device& device, const PhysicalDevice& physicalDevice,
                    vk::DeviceSize size, vk::BufferUsageFlags usage,
                    vk::MemoryPropertyFlags properties);
    void copy(void* data);
    vk::WriteDescriptorSet createWrite();
    uint64_t getDeviceAddress() const { return deviceAddress; }

private:
    void create(vk::Device device, vk::DeviceSize size, vk::BufferUsageFlags usage);
    void allocate(vk::Device device, const PhysicalDevice& physicalDevice,
                  vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties);

    vk::Device device;

    vk::UniqueBuffer buffer;
    vk::UniqueDeviceMemory memory;
    vk::DeviceSize size;
    void* mapped = nullptr;

    uint64_t deviceAddress;
    vk::DescriptorBufferInfo bufferInfo;
};
