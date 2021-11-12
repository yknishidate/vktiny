#pragma once
#include "vktiny/Buffer.hpp"
#include "vktiny/Image.hpp"
#include "vktiny/AccelStruct.hpp"
#include "vktiny/Context.hpp"
#include "vktiny/DescriptorSet.hpp"
#include <unordered_map>

namespace vkt
{
    class DescriptorPool
    {
    public:
        DescriptorPool() = default;
        DescriptorPool(const DescriptorPool&) = delete;
        DescriptorPool(DescriptorPool&&) = default;
        DescriptorPool& operator=(const DescriptorPool&) = delete;
        DescriptorPool& operator=(DescriptorPool&&) = default;

        void initialize(const Context& context,
                        uint32_t maxSets,
                        std::vector<vk::DescriptorPoolSize> poolSizes)
        {
            this->context = &context;

            vk::DescriptorPoolCreateInfo poolInfo;
            poolInfo.setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet);
            poolInfo.setMaxSets(maxSets);
            poolInfo.setPoolSizes(poolSizes);
            descPool = context.getDevice().createDescriptorPoolUnique(poolInfo);
        }

        vk::UniqueDescriptorSetLayout createDescSetLayout(
            const std::vector<vk::DescriptorSetLayoutBinding>& bindings)
        {
            return context->getDevice().createDescriptorSetLayoutUnique({ {}, bindings });
        }

        DescriptorSet createDescSet(vk::DescriptorSetLayout layout)
        {
            DescriptorSet descSet;
            descSet.initialize(*context, *descPool, layout);
            return std::move(descSet);
        }

    private:
        const Context* context;

        vk::UniqueDescriptorPool descPool;
    };
}
