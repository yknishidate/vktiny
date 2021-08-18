#include "vktiny/Vulkan/DescriptorManager.hpp"

namespace vkt
{
    void DescriptorManager::initialize(const Context& context)
    {
        this->device = &context.getDevice();
        this->physicalDevice = &context.getPhysicalDevice();
    }

    void DescriptorManager::prepare(uint32_t maxSets)
    {
        createDescriptorPool(maxSets);
        createDescSetLayout();
        descSets = device->get().allocateDescriptorSetsUnique({ *descPool, *descSetLayout });
        updateDescSets();
    }

    void DescriptorManager::addDescriptors(vk::DescriptorType type,
                                           vk::WriteDescriptorSet write,
                                           uint32_t binding,
                                           uint32_t count)
    {
        // Count desc type
        if (descCount.contains(type)) {
            descCount[type] += count;
        } else {
            descCount[type] = count;
        }

        // Add bindings
        bindings.emplace_back(binding, type, count, vkSS::eAll);

        // Add desc set write
        write.setDstBinding(binding);
        write.setDescriptorType(type);
        descWrites.push_back(write);
    }

    void DescriptorManager::createDescriptorPool(uint32_t maxSets)
    {
        std::vector<vk::DescriptorPoolSize> sizes;
        for (auto& [type, count] : descCount) {
            sizes.emplace_back(type, count);
        }

        vk::DescriptorPoolCreateInfo poolInfo;
        poolInfo.setPoolSizes(sizes);
        poolInfo.setMaxSets(maxSets);
        poolInfo.setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet);
        descPool = device->get().createDescriptorPoolUnique(poolInfo);
    }

    void DescriptorManager::createDescSetLayout()
    {
        vk::DescriptorSetLayoutCreateInfo layoutInfo;
        layoutInfo.setBindings(bindings);
        descSetLayout = device->get().createDescriptorSetLayoutUnique(layoutInfo);
    }

    void DescriptorManager::updateDescSets(uint32_t descSetIndex)
    {
        for (auto& write : descWrites) {
            write.setDstSet(*descSets[descSetIndex]);
        }
        device->get().updateDescriptorSets(descWrites, nullptr);
    }
}