#pragma once
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>
#include <unordered_map>

namespace vkt
{
    class Context;
    class DescriptorPool;
    class DescriptorSetLayout;
    class Buffer;
    class Image;

    class DescriptorSet
    {
    public:
        DescriptorSet(const Context& context,
                      const DescriptorPool& descPool,
                      const DescriptorSetLayout& layout);
        DescriptorSet(const DescriptorSet&) = delete;
        DescriptorSet(DescriptorSet&&) = default;
        DescriptorSet& operator=(const DescriptorSet&) = delete;
        DescriptorSet& operator=(DescriptorSet&&) = default;

        void update(const Buffer& buffer, vk::DescriptorSetLayoutBinding binding);
        void update(const Image& image, vk::DescriptorSetLayoutBinding binding);

        vk::DescriptorSet get() const { return *descSet; }

    private:
        const Context* context;

        vk::UniqueDescriptorSet descSet;
    };
}
