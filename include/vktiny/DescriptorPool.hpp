#pragma once

#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>
#include <unordered_map>

namespace vkt
{
    class Context;

    class DescriptorPool
    {
    public:
        DescriptorPool(const Context& context,
                       uint32_t maxSets,
                       std::vector<vk::DescriptorPoolSize> poolSizes);
        DescriptorPool(const DescriptorPool&) = delete;
        DescriptorPool(DescriptorPool&&) = default;
        DescriptorPool& operator=(const DescriptorPool&) = delete;
        DescriptorPool& operator=(DescriptorPool&&) = default;

        vk::DescriptorPool get() const { return *descPool; }

    private:
        const Context* context;

        vk::UniqueDescriptorPool descPool;
    };
}
