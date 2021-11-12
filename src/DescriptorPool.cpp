#include "vktiny/DescriptorPool.hpp"
#include "vktiny/Context.hpp"

namespace vkt
{
    DescriptorPool::DescriptorPool(const Context& context,
                                   uint32_t maxSets,
                                   std::vector<vk::DescriptorPoolSize> poolSizes)
        : context(&context)
    {
        vk::DescriptorPoolCreateInfo poolInfo;
        poolInfo.setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet);
        poolInfo.setMaxSets(maxSets);
        poolInfo.setPoolSizes(poolSizes);
        descPool = context.getDevice().createDescriptorPoolUnique(poolInfo);
    }

    vk::UniqueDescriptorSetLayout DescriptorPool::createDescSetLayout(const std::vector<vk::DescriptorSetLayoutBinding>& bindings)
    {
        return context->getDevice().createDescriptorSetLayoutUnique({ {}, bindings });
    }

    //DescriptorSet DescriptorPool::createDescSet(vk::DescriptorSetLayout layout)
    //{
    //    DescriptorSet descSet(*context, *descPool, layout);
    //    return std::move(descSet);
    //}
}
