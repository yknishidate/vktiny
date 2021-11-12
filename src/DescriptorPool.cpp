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
}
