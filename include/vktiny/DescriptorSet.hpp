#pragma once
#include "vktiny/Buffer.hpp"
#include "vktiny/Image.hpp"
#include "vktiny/Context.hpp"
#include "vktiny/Pipeline.hpp"
#include <unordered_map>

namespace vkt
{
    class DescriptorSet
    {
    public:
        DescriptorSet() = default;
        DescriptorSet(const DescriptorSet&) = delete;
        DescriptorSet(DescriptorSet&&) = default;
        DescriptorSet& operator=(const DescriptorSet&) = delete;
        DescriptorSet& operator=(DescriptorSet&&) = default;

        void initialize(const Context& context,
                        vk::DescriptorPool descPool,
                        vk::DescriptorSetLayout layout)
        {
            this->context = &context;

            vk::DescriptorSetAllocateInfo allocInfo{ descPool, layout };
            descSet = std::move(context.getDevice().allocateDescriptorSetsUnique(allocInfo).front());
        }

        void update(const Buffer& buffer, vk::DescriptorSetLayoutBinding binding)
        {
            vk::DescriptorBufferInfo bufferInfo(buffer.get(), 0, buffer.getSize());
            vk::WriteDescriptorSet writeDescSet;
            writeDescSet.setDstSet(*descSet);
            writeDescSet.setDstBinding(binding.binding);
            writeDescSet.setDescriptorCount(binding.descriptorCount);
            writeDescSet.setDescriptorType(binding.descriptorType);
            writeDescSet.setBufferInfo(bufferInfo);
            context->getDevice().updateDescriptorSets(writeDescSet, nullptr);
        }

        void update(const Image& image, vk::DescriptorSetLayoutBinding binding)
        {
            vk::DescriptorImageInfo imageInfo({}, image.getView(), image.getLayout());
            vk::WriteDescriptorSet writeDescSet;
            writeDescSet.setDstSet(*descSet);
            writeDescSet.setDstBinding(binding.binding);
            writeDescSet.setDescriptorCount(binding.descriptorCount);
            writeDescSet.setDescriptorType(binding.descriptorType);
            writeDescSet.setImageInfo(imageInfo);
            context->getDevice().updateDescriptorSets(writeDescSet, nullptr);
        }

        void bind(vk::CommandBuffer commandBuffer, const Pipeline& pipeline)
        {
            vk::PipelineBindPoint bindPoint = pipeline.getBindPoint();
            vk::PipelineLayout layout = pipeline.getLayout();
            commandBuffer.bindDescriptorSets(bindPoint, layout, 0, *descSet, nullptr);
        }

    private:
        const Context* context;

        vk::UniqueDescriptorSet descSet;
    };
}
