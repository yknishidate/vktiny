#include "Buffer.hpp"

void Buffer::initialize(const Device& device,
                        const PhysicalDevice& physicalDevice,
                        vk::DeviceSize size,
                        vk::BufferUsageFlags usage,
                        vk::MemoryPropertyFlags properties)
{
    this->device = device.get();
    this->size = size;
    create(device.get(), size, usage);
    allocate(device.get(), physicalDevice, usage, properties);
}

void Buffer::copy(void* data)
{
    if (!mapped) {
        mapped = device.mapMemory(*memory, 0, size);
    }
    memcpy(mapped, data, static_cast<size_t>(size));
}

void Buffer::create(vk::Device device, vk::DeviceSize size, vk::BufferUsageFlags usage)
{
    buffer = device.createBufferUnique({ {}, size, usage });
}

void Buffer::allocate(vk::Device device, const PhysicalDevice& physicalDevice,
                      vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties)
{
    auto requirements = device.getBufferMemoryRequirements(*buffer);
    auto memoryTypeIndex = physicalDevice.findMemoryType(requirements.memoryTypeBits, properties);
    vk::MemoryAllocateInfo allocInfo{ requirements.size, memoryTypeIndex };

    if (usage & vk::BufferUsageFlagBits::eShaderDeviceAddress) {
        vk::MemoryAllocateFlagsInfo flagsInfo{ vk::MemoryAllocateFlagBits::eDeviceAddress };
        allocInfo.pNext = &flagsInfo;

        memory = device.allocateMemoryUnique(allocInfo);
        device.bindBufferMemory(*buffer, *memory, 0);

        vk::BufferDeviceAddressInfoKHR bufferDeviceAddressInfo{ *buffer };
        deviceAddress = device.getBufferAddressKHR(&bufferDeviceAddressInfo);
    } else {
        memory = device.allocateMemoryUnique(allocInfo);
        device.bindBufferMemory(*buffer, *memory, 0);
    }
}

vk::WriteDescriptorSet Buffer::createWrite()
{
    bufferInfo = vk::DescriptorBufferInfo{ *buffer, 0, size };

    vk::WriteDescriptorSet bufferWrite;
    bufferWrite.setDescriptorCount(1);
    bufferWrite.setBufferInfo(bufferInfo);
    return bufferWrite;
}
