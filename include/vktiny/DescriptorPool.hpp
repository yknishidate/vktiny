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
            vk::DescriptorPoolCreateInfo poolInfo;
            poolInfo.setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet);
            poolInfo.setMaxSets(maxSets);
            poolInfo.setPoolSizes(poolSizes);
            descPool = vk::raii::DescriptorPool(context.getDevice(), poolInfo);
        }

        const vk::raii::DescriptorPool& get() const { return descPool; }

    protected:
        vk::raii::DescriptorPool descPool = nullptr;
    };
}
