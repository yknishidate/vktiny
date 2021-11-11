#pragma once
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>

namespace vkt
{
    class Instance
    {
    public:
        Instance() = default;
        Instance(const Instance&) = delete;
        Instance(Instance&&) = default;
        Instance& operator = (const Instance&) = delete;
        Instance& operator = (Instance&&) = default;

        void initialize(uint32_t apiVersion,
                        std::vector<const char*> layers = {},
                        std::vector<const char*> extensions = {});

        vk::Instance get() const { return instance.get(); }

    private:
        vk::UniqueInstance instance;
    };
}
