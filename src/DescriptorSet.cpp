#include "vktiny/DescriptorSet.hpp"
#include "vktiny/Buffer.hpp"
#include "vktiny/Image.hpp"
#include "vktiny/Pipeline.hpp"

namespace vkt
{
    DescriptorSet::DescriptorSet(const Context& context,
                                 vk::DescriptorPool descPool,
                                 vk::DescriptorSetLayout layout)
        : context(&context)
    {
        vk::DescriptorSetAllocateInfo allocInfo{ descPool, layout };
        descSet = std::move(context.getDevice().allocateDescriptorSetsUnique(allocInfo).front());
    }

    //void DescriptorSet::update(const Buffer& buffer, vk::DescriptorSetLayoutBinding binding)
    //{
    //    vk::DescriptorBufferInfo bufferInfo(buffer.get(), 0, buffer.getSize());
    //    vk::WriteDescriptorSet writeDescSet;
    //    writeDescSet.setDstSet(*descSet);
    //    writeDescSet.setDstBinding(binding.binding);
    //    writeDescSet.setDescriptorCount(binding.descriptorCount);
    //    writeDescSet.setDescriptorType(binding.descriptorType);
    //    writeDescSet.setBufferInfo(bufferInfo);
    //    context->getDevice().updateDescriptorSets(writeDescSet, nullptr);
    //}

    void DescriptorSet::update(const Image& image, vk::DescriptorSetLayoutBinding binding)
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
}
