#include "vktiny/Context.hpp"
#include "vktiny/DescriptorSetLayout.hpp"

namespace vkt
{
    DescriptorSetLayout::DescriptorSetLayout(
        const Context& context,
        const std::vector<vk::DescriptorSetLayoutBinding>& bindings)
    {
        descSetLayout = context.getDevice().createDescriptorSetLayoutUnique({ {}, bindings });
    }
}
