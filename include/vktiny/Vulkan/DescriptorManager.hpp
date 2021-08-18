#pragma once
#include "vktiny/Vulkan/Buffer.hpp"
#include "vktiny/Vulkan/Image.hpp"
#include "vktiny/Vulkan/AccelStruct.hpp"
#include "vktiny/Vulkan/Context.hpp"
#include <unordered_map>

namespace vkt
{
    class DescriptorManager
    {
    public:
        using vkDT = vk::DescriptorType;
        using vkSS = vk::ShaderStageFlagBits;

        void initialize(const Context& context);

        void prepare(uint32_t maxSets = 1);

        // TODO: create multi sets
        void addStorageBuffer(Buffer& buffer, uint32_t binding, uint32_t set = 0)
        {
            addDescriptors(vkDT::eStorageBuffer, buffer.createWrite(), binding, 1);
        }

        void addUniformBuffer(Buffer& buffer, uint32_t binding, uint32_t set = 0)
        {
            addDescriptors(vkDT::eUniformBuffer, buffer.createWrite(), binding, 1);
        }

        void addStorageImage(Image& image, uint32_t binding, uint32_t set = 0)
        {
            addDescriptors(vkDT::eStorageImage, image.createWrite(), binding, 1);
        }

        void addTopLevelAccelStruct(TopLevelAccelStruct& topLevelAS,
                                    uint32_t binding, uint32_t set = 0)
        {
            addDescriptors(vkDT::eAccelerationStructureKHR, topLevelAS.createWrite(), binding, 1);
        }

        void addCombinedImageSamplers(std::vector<Image>& images,
                                      uint32_t binding, uint32_t set = 0)
        {
            // TODO: reserve info
            std::vector<vk::DescriptorImageInfo> info;
            for (auto& image : images) {
                info.push_back(image.getDescInfo());
            }
            vk::WriteDescriptorSet write;
            write.setDescriptorCount(images.size());
            write.setImageInfo(info);
            addDescriptors(vkDT::eCombinedImageSampler, write, binding, images.size());
        }

        const auto& getDescSet() const { return *descSets.front(); }
        const auto& getDescSetLayout() const { return *descSetLayout; }

    private:
        void addDescriptors(vk::DescriptorType type, vk::WriteDescriptorSet write,
                            uint32_t binding, uint32_t count);

        const Device* device;
        const PhysicalDevice* physicalDevice;

        // Descriptor
        void createDescriptorPool(uint32_t maxSets);
        void createDescSetLayout();
        void updateDescSets(uint32_t descSetIndex = 0);

        vk::UniqueDescriptorPool descPool;
        std::vector<vk::UniqueDescriptorSet> descSets;
        vk::UniqueDescriptorSetLayout descSetLayout;
        std::vector<vk::DescriptorSetLayoutBinding> bindings;
        std::unordered_map<vk::DescriptorType, uint32_t> descCount;
        std::vector<vk::WriteDescriptorSet> descWrites;
    };
}
