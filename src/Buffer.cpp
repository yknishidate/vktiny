#include "Buffer.hpp"

void Buffer::initialize(const Context& context,
                        vk::DeviceSize size,
                        vk::BufferUsageFlags usage,
                        vk::MemoryPropertyFlags properties,
                        void* data)
{
    this->context = &context;
    this->size = size;
    create(size, usage);
    allocate(usage, properties);
    if (data) {
        copy(data);
    }
}

void Buffer::copy(void* data)
{
    if (!mapped) {
        mapped = context->getVkDevice().mapMemory(*memory, 0, size);
    }
    memcpy(mapped, data, static_cast<size_t>(size));
}

void Buffer::create(vk::DeviceSize size, vk::BufferUsageFlags usage)
{
    buffer = context->getVkDevice().createBufferUnique({ {}, size, usage });
}

void Buffer::allocate(vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties)
{
    auto requirements = context->getVkDevice().getBufferMemoryRequirements(*buffer);
    auto memoryTypeIndex = context->getPhysicalDevice().findMemoryType(
        requirements.memoryTypeBits, properties);
    vk::MemoryAllocateInfo allocInfo{ requirements.size, memoryTypeIndex };

    if (usage & vk::BufferUsageFlagBits::eShaderDeviceAddress) {
        vk::MemoryAllocateFlagsInfo flagsInfo{ vk::MemoryAllocateFlagBits::eDeviceAddress };
        allocInfo.pNext = &flagsInfo;

        memory = context->getVkDevice().allocateMemoryUnique(allocInfo);
        context->getVkDevice().bindBufferMemory(*buffer, *memory, 0);

        vk::BufferDeviceAddressInfoKHR bufferDeviceAddressInfo{ *buffer };
        deviceAddress = context->getVkDevice().getBufferAddressKHR(&bufferDeviceAddressInfo);
    } else {
        memory = context->getVkDevice().allocateMemoryUnique(allocInfo);
        context->getVkDevice().bindBufferMemory(*buffer, *memory, 0);
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
