#pragma once
#include "Context.hpp"
#include "DescriptorPool.hpp"
#include "DescriptorSetLayout.hpp"
#include "Buffer.hpp"
#include "Image.hpp"

namespace vkt
{
    class DescriptorSet
    {
    public:
        DescriptorSet() = default;
        DescriptorSet(const DescriptorSet&) = delete;
        DescriptorSet& operator=(const DescriptorSet&) = delete;

        void initialize(const Context& context,
                        const DescriptorPool& descPool,
                        const DescriptorSetLayout& layout)
        {
            vk::DescriptorSetAllocateInfo allocInfo(*descPool.get(), *layout.get());
            descSet = std::move(vk::raii::DescriptorSets(context.getDevice(), allocInfo).front());
        }

        void update(const Context& context,
                    const Buffer& buffer,
                    vk::DescriptorSetLayoutBinding binding)
        {
            vk::DescriptorBufferInfo bufferInfo(*buffer.get(), 0, buffer.getSize());
            vk::WriteDescriptorSet writeDescSet;
            writeDescSet.setDstSet(*descSet);
            writeDescSet.setDstBinding(binding.binding);
            writeDescSet.setDescriptorCount(binding.descriptorCount);
            writeDescSet.setDescriptorType(binding.descriptorType);
            writeDescSet.setBufferInfo(bufferInfo);
            context.getDevice().updateDescriptorSets(writeDescSet, nullptr);
        }

        const vk::raii::DescriptorSet& get() const { return descSet; }

    protected:
        vk::raii::DescriptorSet descSet = nullptr;
    };

}
