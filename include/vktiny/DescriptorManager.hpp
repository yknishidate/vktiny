#pragma once
#include "vktiny/Buffer.hpp"
#include "vktiny/Image.hpp"
#include "vktiny/AccelStruct.hpp"
#include "vktiny/Context.hpp"
#include <unordered_map>

namespace vkt
{
    class DescriptorManager
    {
    public:
        DescriptorManager() = default;
        DescriptorManager(const DescriptorManager&) = delete;
        DescriptorManager(DescriptorManager&&) = default;
        DescriptorManager& operator=(const DescriptorManager&) = delete;
        DescriptorManager& operator=(DescriptorManager&&) = default;

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

        void addTopLevelAccelStructs(std::vector<TopLevelAccelStruct>& topLevelASs,
                                     uint32_t binding, uint32_t set = 0)
        {
            accelStructInfos.emplace_back();
            for (auto& topLevelAS : topLevelASs) {
                accelStructInfos.back().push_back(topLevelAS.getAccelStructInfo());
            }

            vk::WriteDescriptorSet write;
            write.setDescriptorCount(topLevelASs.size());
            write.setPNext(accelStructInfos.back().data());
            addDescriptors(vkDT::eAccelerationStructureKHR, write, binding, topLevelASs.size());
        }

        void addCombinedImageSamplers(std::vector<Image>& images,
                                      uint32_t binding, uint32_t set = 0)
        {
            imageInfos.emplace_back();
            for (auto& image : images) {
                imageInfos.back().push_back(image.getDescInfo());
            }
            vk::WriteDescriptorSet write;
            write.setDescriptorCount(images.size());
            write.setImageInfo(imageInfos.back());
            addDescriptors(vkDT::eCombinedImageSampler, write, binding, images.size());
        }

        const auto& getDescSet() const { return *descSets.front(); }
        const auto& getDescSetLayout() const { return *descSetLayout; }

    private:
        void addDescriptors(vk::DescriptorType type, vk::WriteDescriptorSet write,
                            uint32_t binding, uint32_t count);

        void createDescriptorPool(uint32_t maxSets);
        void createDescSetLayout();
        void updateDescSets(uint32_t descSetIndex = 0);

        const Context* context;

        template <class T>
        using VectorList = std::list<std::vector<T>>;
        VectorList<vk::DescriptorBufferInfo> bufferInfos;
        VectorList<vk::DescriptorImageInfo> imageInfos;
        VectorList<vk::WriteDescriptorSetAccelerationStructureKHR> accelStructInfos;

        vk::UniqueDescriptorPool descPool;
        std::vector<vk::UniqueDescriptorSet> descSets;
        vk::UniqueDescriptorSetLayout descSetLayout;
        std::vector<vk::DescriptorSetLayoutBinding> bindings;
        std::unordered_map<vk::DescriptorType, uint32_t> descCount;
        std::vector<vk::WriteDescriptorSet> descWrites;
    };
}
