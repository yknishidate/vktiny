#pragma once
#include "Context.hpp"

namespace vkt
{
    class DescriptorPool
    {
    public:
        DescriptorPool() = default;
        DescriptorPool(const DescriptorPool&) = delete;
        DescriptorPool& operator=(const DescriptorPool&) = delete;

        void initialize(const Context& context,
                        uint32_t maxSets,
                        const std::vector<vk::DescriptorPoolSize>& poolSizes)
        {
            descPool = vk::raii::DescriptorPool(context.getDevice(), { {}, maxSets, poolSizes });

            //vk::DescriptorSetAllocateInfo{}
            //descSets = context.getDevice().allocateDescriptorSets({*descPool, *descSetLayout});
        }

    protected:
        vk::raii::DescriptorPool descPool;
        std::vector<vk::raii::DescriptorSet> descSets;
    };

}
