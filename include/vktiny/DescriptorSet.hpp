#pragma once
#include "Context.hpp"
#include "DescriptorPool.hpp"
#include "DescriptorSetLayout.hpp"

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

        const vk::raii::DescriptorSet& get() const { return descSet; }

    protected:
        vk::raii::DescriptorSet descSet = nullptr;
    };

}
