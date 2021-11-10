#include "Context.hpp"

namespace vkt
{
    class Buffer
    {
    public:
        Buffer() = default;
        Buffer(const Buffer&) = delete;
        Buffer& operator = (const Buffer&) = delete;

        using vkMP = vk::MemoryPropertyFlagBits;
        void initialize(const Context& context,
                        vk::DeviceSize size,
                        vk::BufferUsageFlags usage,
                        vk::MemoryPropertyFlags propertyFlags = vkMP::eHostVisible | vkMP::eHostCoherent)
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

    private:
        vk::raii::Buffer buffer = nullptr;
        vk::raii::DeviceMemory deviceMemory = nullptr;
    };
}
