#pragma once
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>

class Instance
{
public:
    void initialize(uint32_t apiVersion,
                    std::vector<const char*> layers = {},
                    std::vector<const char*> extensions = {});

    vk::Instance get() const { return instance.get(); }

private:
    vk::UniqueInstance instance;
};
