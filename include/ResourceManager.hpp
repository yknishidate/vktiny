#pragma once
#include "Buffer.hpp"
#include "Image.hpp"
#include "AccelStruct.hpp"
#include <unordered_map>
#include "Context.hpp"

class ResourceManager
{
public:
    void initialize(const Context& context)
    {
        this->device = &context.getDevice();
        this->physicalDevice = &context.getPhysicalDevice();
    }

    void prepare(uint32_t maxSets = 1)
    {
        createDescriptorPool(maxSets);
        createDescSetLayout();
        descSets = device->get().allocateDescriptorSetsUnique({ *descPool, *descSetLayout });
        updateDescSets();
    }

    using vkDT = vk::DescriptorType;
    using vkSS = vk::ShaderStageFlagBits;
    Image& addStorageImage(vk::Extent2D extent, vk::Format format, vk::ImageUsageFlags usage,
                           vk::ImageLayout imageLayout = vk::ImageLayout::eUndefined)
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

    Buffer& addUniformBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage,
                             vk::MemoryPropertyFlags properties, void* data = nullptr)
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

    Buffer& addStorageBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage,
                             vk::MemoryPropertyFlags properties, void* data = nullptr)
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

    AccelStruct& addAccelStruct()
    {

    }

    //Mesh& addMesh();

    const auto& getDescSet() const { return *descSets.front(); }
    const auto& getDescSetLayout() const { return *descSetLayout; }

private:
    void addDescriptor(vk::DescriptorType type, vk::WriteDescriptorSet write)
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

    const Device* device;
    const PhysicalDevice* physicalDevice;

    std::vector<Image> storageImages;
    std::vector<Buffer> uniformBuffers;
    std::vector<Buffer> storageBuffers;
    std::vector<AccelStruct> accelStructs;

    //std::vector<Mesh> meshes;
    //std::vector<Material> materials;
    //std::vector<Texture> textures;

    // Descriptor
    void createDescriptorPool(uint32_t maxSets)
    {
        std::vector<vk::DescriptorPoolSize> sizes;
        for (auto& pair : descCount) {
            sizes.emplace_back(pair.first, pair.second);
        }

        vk::DescriptorPoolCreateInfo poolInfo;
        poolInfo.setPoolSizes(sizes);
        poolInfo.setMaxSets(maxSets);
        poolInfo.setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet);
        descPool = device->get().createDescriptorPoolUnique(poolInfo);
    }

    void createDescSetLayout()
    {
        vk::DescriptorSetLayoutCreateInfo layoutInfo;
        layoutInfo.setBindings(bindings);
        descSetLayout = device->get().createDescriptorSetLayoutUnique(layoutInfo);
    }

    void updateDescSets(uint32_t descSetIndex = 0)
    {
        for (auto& write : descWrites) {
            write.setDstSet(*descSets[descSetIndex]);
        }
        device->get().updateDescriptorSets(descWrites, nullptr);
    }

    int currentBinding = 0;
    vk::UniqueDescriptorPool descPool;
    std::vector<vk::UniqueDescriptorSet> descSets;
    vk::UniqueDescriptorSetLayout descSetLayout;
    std::vector<vk::DescriptorSetLayoutBinding> bindings;
    std::unordered_map<vk::DescriptorType, uint32_t> descCount;
    std::vector<vk::WriteDescriptorSet> descWrites;
};
