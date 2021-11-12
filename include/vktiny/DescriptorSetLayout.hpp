#pragma once
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>
#include <unordered_map>

namespace vkt
{
    class Context;
    class Buffer;
    class Image;

    class DescriptorSetLayout
    {
    public:
        DescriptorSetLayout(const Context& context,
                            const std::vector<vk::DescriptorSetLayoutBinding>& bindings);
        DescriptorSetLayout(const DescriptorSetLayout&) = delete;
        DescriptorSetLayout(DescriptorSetLayout&&) = default;
        DescriptorSetLayout& operator=(const DescriptorSetLayout&) = delete;
        DescriptorSetLayout& operator=(DescriptorSetLayout&&) = default;

        vk::DescriptorSetLayout get() const { return *descSetLayout; }

    private:
        const Context* context;

        vk::UniqueDescriptorSetLayout descSetLayout;
    };
}
