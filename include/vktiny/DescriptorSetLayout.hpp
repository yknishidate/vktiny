#pragma once
#include "Context.hpp"

namespace vkt
{
    class DescriptorSetLayout
    {
    public:
        DescriptorSetLayout() = default;
        DescriptorSetLayout(const DescriptorSetLayout&) = delete;
        DescriptorSetLayout& operator=(const DescriptorSetLayout&) = delete;

        void initialize(const Context& context,
                        const std::vector<vk::DescriptorSetLayoutBinding>& bindings)
        {
            layout = vk::raii::DescriptorSetLayout(context.getDevice(), { {}, bindings });
        }

        const vk::raii::DescriptorSetLayout& get() const { return layout; }

    protected:
        vk::raii::DescriptorSetLayout layout = nullptr;
    };

}
