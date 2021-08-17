#include "ResourceManager.hpp"

void ResourceManager::initialize(const Context& context)
{
    this->device = &context.getDevice();
    this->physicalDevice = &context.getPhysicalDevice();
}

void ResourceManager::prepare(uint32_t maxSets)
{
    createDescriptorPool(maxSets);
    createDescSetLayout();
    descSets = device->get().allocateDescriptorSetsUnique({ *descPool, *descSetLayout });
    updateDescSets();
}

using vkDT = vk::DescriptorType;
using vkSS = vk::ShaderStageFlagBits;
Image& ResourceManager::addStorageImage(vk::Extent2D extent,
                                        vk::Format format,
                                        vk::ImageUsageFlags usage,
                                        vk::ImageLayout imageLayout)
{
    storageImages.emplace_back();

    Image& image = storageImages.back();
    image.initialize(*device, *physicalDevice, extent, format, usage);
    image.createImageView();
    if (imageLayout != vk::ImageLayout::eUndefined) {
        image.transitionImageLayout(vk::ImageLayout::eGeneral);
    }
    addDescriptor(vkDT::eStorageImage, image.createWrite());
    return image;
}

Buffer& ResourceManager::addUniformBuffer(vk::DeviceSize size,
                                          vk::BufferUsageFlags usage,
                                          vk::MemoryPropertyFlags properties,
                                          void* data)
{
    uniformBuffers.emplace_back();

    Buffer& buffer = uniformBuffers.back();
    buffer.initialize(*device, *physicalDevice, size, usage, properties);
    if (data) {
        buffer.copy(data);
    }
    addDescriptor(vkDT::eUniformBuffer, buffer.createWrite());
    return buffer;
}

Buffer& ResourceManager::addStorageBuffer(vk::DeviceSize size,
                                          vk::BufferUsageFlags usage,
                                          vk::MemoryPropertyFlags properties,
                                          void* data)
{
    storageBuffers.emplace_back();
    Buffer& buffer = storageBuffers.back();
    buffer.initialize(*device, *physicalDevice, size, usage, properties);
    if (data) {
        buffer.copy(data);
    }
    addDescriptor(vkDT::eStorageBuffer, buffer.createWrite());
    return buffer;
}

void ResourceManager::addDescriptor(vk::DescriptorType type, vk::WriteDescriptorSet write)
{
    // Count desc type
    if (descCount.contains(type)) {
        descCount[type] += 1;
    } else {
        descCount[type] = 1;
    }

    // Add bindings
    bindings.emplace_back(currentBinding, type, 1, vkSS::eAll);

    // Add desc set write
    write.setDstBinding(currentBinding);
    write.setDescriptorType(type);
    descWrites.push_back(write);

    currentBinding++;
}

void ResourceManager::createDescriptorPool(uint32_t maxSets)
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

void ResourceManager::createDescSetLayout()
{
    vk::DescriptorSetLayoutCreateInfo layoutInfo;
    layoutInfo.setBindings(bindings);
    descSetLayout = device->get().createDescriptorSetLayoutUnique(layoutInfo);
}

void ResourceManager::updateDescSets(uint32_t descSetIndex)
{
    for (auto& write : descWrites) {
        write.setDstSet(*descSets[descSetIndex]);
    }
    device->get().updateDescriptorSets(descWrites, nullptr);
}
