#pragma once
#include "Buffer.hpp"
#include "Image.hpp"
#include "AccelStruct.hpp"
#include <unordered_map>
#include "Context.hpp"

class ResourceManager
{
public:
    void initialize(const Context& context);

    void prepare(uint32_t maxSets = 1);

    using vkDT = vk::DescriptorType;
    using vkSS = vk::ShaderStageFlagBits;
    Image& addStorageImage(vk::Extent2D extent,
                           vk::Format format,
                           vk::ImageUsageFlags usage,
                           vk::ImageLayout imageLayout = vk::ImageLayout::eUndefined);

    Buffer& addUniformBuffer(vk::DeviceSize size,
                             vk::BufferUsageFlags usage,
                             vk::MemoryPropertyFlags properties,
                             void* data = nullptr);

    Buffer& addStorageBuffer(vk::DeviceSize size,
                             vk::BufferUsageFlags usage,
                             vk::MemoryPropertyFlags properties,
                             void* data = nullptr);

    TopLevelAccelStruct& addTopLevelAccelStruct();

    //AccelStruct& addAccelStruct(const std::vector<Vertex>& vertices, const Buffer& vertexBuffer,
    //                            const std::vector<Index>& indices, const Buffer& indexBuffer)
    //{
    //    vk::AccelerationStructureGeometryTrianglesDataKHR triangleData;
    //    triangleData.setVertexFormat(vk::Format::eR32G32B32Sfloat);
    //    triangleData.setVertexData(vertexBuffer.getDeviceAddress());
    //    triangleData.setVertexStride(sizeof(Vertex));
    //    triangleData.setMaxVertex(vertices.size());
    //    triangleData.setIndexType(vk::IndexType::eUint32);
    //    triangleData.setIndexData(indexBuffer.getDeviceAddress());

    //    vk::AccelerationStructureGeometryKHR geometry;
    //    geometry.setGeometryType(vk::GeometryTypeKHR::eTriangles);
    //    geometry.setGeometry({ triangleData });
    //    geometry.setFlags(vk::GeometryFlagBitsKHR::eOpaque);

    //    uint32_t primitiveCount = indices.size() / 3;
    //    bottomLevelAS.createBuffer(*device, physicalDevice, geometry,
    //                               vk::AccelerationStructureTypeKHR::eBottomLevel, primitiveCount);
    //    bottomLevelAS.create();
    //    vk::UniqueCommandBuffer cmdBuf = createCommandBuffer();
    //    bottomLevelAS.build(*cmdBuf);
    //    submitCommandBuffer(*cmdBuf);
    //}

    //Mesh& addMesh();

    const auto& getDescSet() const { return *descSets.front(); }
    const auto& getDescSetLayout() const { return *descSetLayout; }

private:
    void addDescriptor(vk::DescriptorType type, vk::WriteDescriptorSet write);

    const Device* device;
    const PhysicalDevice* physicalDevice;

    std::list<Image> storageImages;
    std::list<Buffer> uniformBuffers;
    std::list<Buffer> storageBuffers;
    std::list<AccelStruct> accelStructs;

    //std::vector<Mesh> meshes;
    //std::vector<Material> materials;
    //std::vector<Texture> textures;

    // Descriptor
    void createDescriptorPool(uint32_t maxSets);
    void createDescSetLayout();
    void updateDescSets(uint32_t descSetIndex = 0);

    int currentBinding = 0;
    vk::UniqueDescriptorPool descPool;
    std::vector<vk::UniqueDescriptorSet> descSets;
    vk::UniqueDescriptorSetLayout descSetLayout;
    std::vector<vk::DescriptorSetLayoutBinding> bindings;
    std::unordered_map<vk::DescriptorType, uint32_t> descCount;
    std::vector<vk::WriteDescriptorSet> descWrites;
};
