#pragma once

#include <iostream>

#include <vulkan/vulkan.hpp>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_NOEXCEPTION
#include "tiny_gltf.h"

namespace vkr
{
    class Model;
    struct Scene;
    struct Node;
    struct Mesh;
    struct Material;
    struct Texture;


    struct Vertex
    {
        glm::vec3 pos;

        glm::vec3 normal;

        glm::vec2 uv;

        glm::vec4 color;

        glm::vec4 joint0;

        glm::vec4 weight0;

        glm::vec4 tangent;
    };


    struct Texture
    {
        vk::UniqueImage image;

        vk::UniqueImageView imageView;

        vk::UniqueDeviceMemory deviceMemory;

        vk::DeviceSize deviceSize;

        vk::Extent2D extent;

        vk::Format format;

        vk::ImageLayout imageLayout;

        vk::UniqueSampler sampler;

        uint32_t mipLevels;

        uint32_t layerCount;
    };


    enum class AlphaMode
    {
        Opaque,

        Mask,

        Blend
    };


    struct Material
    {
        // Base color
        int baseColorTexture{ -1 };
        glm::vec4 baseColorFactor{ 1.0f };

        // Metallic / Roughness
        int metallicRoughnessTexture{ -1 };
        float metallicFactor{ 1.0f };
        float roughnessFactor{ 1.0f };

        int normalTexture{ -1 };

        int occlusionTexture{ -1 };

        // Emissive
        int emissiveTexture{ -1 };
        glm::vec3 emissiveFactor{ 0.0f };

        AlphaMode alphaMode{ AlphaMode::Opaque };
        float alphaCutoff{ 0.5f };

        bool doubleSided{ false };
    };


    struct Mesh
    {
        // Vertex
        std::vector<Vertex> vertices;

        vk::UniqueBuffer vertexBuffer;

        vk::UniqueDeviceMemory vertexDeviceMemory;

        vk::DeviceSize vertexDeviceSize;

        // Index
        std::vector<uint32_t> indices;

        vk::UniqueBuffer indexBuffer;

        vk::UniqueDeviceMemory indexDeviceMemory;

        vk::DeviceSize indexDeviceSize;

        int material{ -1 };
    };


    struct Node
    {
        std::vector<int> children;

        int mesh{ -1 };

        glm::mat4 worldMatrix{ 1.0f };

        glm::vec3 translation{ 1.0f };

        glm::quat rotation{ 1.0f, 0.0f, 0.0f, 0.0f };

        glm::vec3 scale{ 1.0f, 1.0f, 1.0f };
    };


    struct Scene
    {
        std::vector<int> nodes;
    };


    class Model final
    {
    public:

        Model() = default;
        ~Model() = default;

        Model(const Model&) = delete;
        Model(Model&&) noexcept = default;
        Model& operator=(const Model&) = delete;
        Model& operator=(Model&&) noexcept = default;

        const std::vector<Scene>& getScenes() const { return scenes; }

        const std::vector<Node>& getNodes() const { return nodes; }

        const std::vector<Mesh>& getMeshes() const { return meshes; }

        const std::vector<Material>& getMaterials() const { return materials; }

        const std::vector<Texture>& getTextures() const { return textures; }

        void loadFromFile(vk::Device device, vk::PhysicalDevice physicalDevice, vk::CommandPool cmdPool, vk::Queue queue, const std::string& filepath);

    private:

        void loadScenes(tinygltf::Model& gltfModel);

        void loadNodes(tinygltf::Model& gltfModel);

        void loadMeshes(tinygltf::Model& gltfModel);

        void loadMaterials(tinygltf::Model& gltfModel);

        void loadTextures(tinygltf::Model& gltfModel);

        void createVertexBuffer(Mesh& mesh, bool raytrace);

        void createIndexBuffer(Mesh& mesh, bool raytrace);

        vk::UniqueCommandBuffer createCommandBuffer();

        void submitCommandBuffer(vk::CommandBuffer cmdBuf);

        vk::Device device;

        vk::PhysicalDevice physicalDevice;

        vk::CommandPool cmdPool;

        vk::Queue queue;

        std::vector<Scene> scenes;

        std::vector<Node> nodes;
        vk::UniqueBuffer nodesBuffer;
        vk::UniqueDeviceMemory nodesDeviceMemory;

        std::vector<Mesh> meshes;
        vk::UniqueBuffer meshesBuffer;
        vk::UniqueDeviceMemory meshesDeviceMemory;

        std::vector<Material> materials;

        std::vector<Texture> textures;

    };
}


namespace vkr
{
    namespace
    {
        uint32_t findMemoryType(vk::PhysicalDevice physicalDevice, const uint32_t typeFilter, const vk::MemoryPropertyFlags properties)
        {
            vk::PhysicalDeviceMemoryProperties memProperties = physicalDevice.getMemoryProperties();

            for (uint32_t i = 0; i != memProperties.memoryTypeCount; ++i) {
                if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                    return i;
                }
            }

            throw std::runtime_error("failed to find suitable memory type");
        }

        struct StagingBuffer
        {
            StagingBuffer(vk::Device device, vk::PhysicalDevice physicalDevice, vk::DeviceSize size, vk::BufferUsageFlags usage, void* data)
            {
                using vkbu = vk::BufferUsageFlagBits;
                using vkmp = vk::MemoryPropertyFlagBits;

                // Remove transferDst
                usage = usage & ~vkbu::eTransferDst;
                // Add transferSrc
                usage = usage | vkbu::eTransferSrc;

                // Create buffer
                buffer = device.createBufferUnique({ {}, size, usage });

                vk::MemoryPropertyFlags memoryProperty{
                    vkmp::eHostVisible | vkmp::eHostCoherent
                };

                // Allocate memory
                const auto requirements = device.getBufferMemoryRequirements(*buffer);
                vk::MemoryAllocateInfo allocInfo{};
                allocInfo.allocationSize = requirements.size;
                allocInfo.memoryTypeIndex = findMemoryType(physicalDevice, requirements.memoryTypeBits, memoryProperty);
                vk::MemoryAllocateFlagsInfo flagsInfo{ vk::MemoryAllocateFlagBits::eDeviceAddress };
                allocInfo.pNext = &flagsInfo;
                memory = device.allocateMemoryUnique(allocInfo);

                // Bind memory to buffer
                device.bindBufferMemory(*buffer, *memory, 0);

                // Copy data
                void* dataPtr = device.mapMemory(*memory, 0, size);
                memcpy(dataPtr, data, static_cast<size_t>(size));
                device.unmapMemory(*memory);
            }

            vk::UniqueBuffer buffer;

            vk::UniqueDeviceMemory memory;
        };

        void createImage(vk::Device device, vk::PhysicalDevice physicalDevice, Texture& tex)
        {
            // Create image
            vk::ImageCreateInfo imageInfo = {};
            imageInfo.imageType = vk::ImageType::e2D;
            imageInfo.extent.width = tex.extent.width;
            imageInfo.extent.height = tex.extent.height;
            imageInfo.extent.depth = 1;
            imageInfo.mipLevels = tex.mipLevels;
            imageInfo.arrayLayers = 1;
            imageInfo.format = tex.format;
            imageInfo.usage = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst;

            tex.image = device.createImageUnique(imageInfo);

            // Allocate memory
            vk::MemoryPropertyFlags properties = vk::MemoryPropertyFlagBits::eDeviceLocal;

            const auto requirements = device.getImageMemoryRequirements(*tex.image);
            auto memoryTypeIndex = findMemoryType(physicalDevice, requirements.memoryTypeBits, properties);
            tex.deviceMemory = device.allocateMemoryUnique({ requirements.size, memoryTypeIndex });

            // Bind memory
            device.bindImageMemory(*tex.image, *tex.deviceMemory, 0);

            // Create image view
            vk::ImageViewCreateInfo createInfo{ {}, *tex.image, vk::ImageViewType::e2D, tex.format };
            createInfo.subresourceRange = { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 };
            tex.imageView = device.createImageViewUnique(createInfo);
        }

        void setImageLayout(vk::CommandBuffer cmdBuf, vk::Image image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout)
        {
            vk::PipelineStageFlags srcStageMask = vk::PipelineStageFlagBits::eAllCommands;
            vk::PipelineStageFlags dstStageMask = vk::PipelineStageFlagBits::eAllCommands;

            vk::ImageMemoryBarrier imageMemoryBarrier{};
            imageMemoryBarrier
                .setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                .setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                .setImage(image)
                .setOldLayout(oldLayout)
                .setNewLayout(newLayout)
                .setSubresourceRange({ vk::ImageAspectFlagBits::eColor , 0, 1, 0, 1 });

            // Source layouts (old)
            switch (oldLayout) {
                case vk::ImageLayout::eUndefined:
                    imageMemoryBarrier.srcAccessMask = {};
                    break;
                case vk::ImageLayout::ePreinitialized:
                    imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eHostWrite;
                    break;
                case vk::ImageLayout::eColorAttachmentOptimal:
                    imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
                    break;
                case vk::ImageLayout::eDepthStencilAttachmentOptimal:
                    imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentWrite;
                    break;
                case vk::ImageLayout::eTransferSrcOptimal:
                    imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
                    break;
                case vk::ImageLayout::eTransferDstOptimal:
                    imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
                    break;
                case vk::ImageLayout::eShaderReadOnlyOptimal:
                    imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eShaderRead;
                    break;
                default:
                    break;
            }

            // Target layouts (new)
            switch (newLayout) {
                case vk::ImageLayout::eTransferDstOptimal:
                    imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
                    break;
                case vk::ImageLayout::eTransferSrcOptimal:
                    imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;
                    break;
                case vk::ImageLayout::eColorAttachmentOptimal:
                    imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
                    break;
                case vk::ImageLayout::eDepthStencilAttachmentOptimal:
                    imageMemoryBarrier.dstAccessMask = imageMemoryBarrier.dstAccessMask | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
                    break;
                case vk::ImageLayout::eShaderReadOnlyOptimal:
                    if (imageMemoryBarrier.srcAccessMask == vk::AccessFlags{}) {
                        imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eHostWrite | vk::AccessFlagBits::eTransferWrite;
                    }
                    imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
                    break;
                default:
                    break;
            }

            cmdBuf.pipelineBarrier(
                srcStageMask,      // srcStageMask
                dstStageMask,      // dstStageMask
                {},                // dependencyFlags
                {},                // memoryBarriers
                {},                // bufferMemoryBarriers
                imageMemoryBarrier // imageMemoryBarriers
            );
        }

        void copyBufferToTexture(vk::CommandBuffer cmdBuf, Texture& tex, vk::Buffer buffer)
        {
            vk::BufferImageCopy region{};
            region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
            region.imageSubresource.mipLevel = 0;
            region.imageSubresource.baseArrayLayer = 0;
            region.imageSubresource.layerCount = 1;
            region.imageExtent = vk::Extent3D{ tex.extent.width, tex.extent.height, 1 };

            cmdBuf.copyBufferToImage(buffer, *tex.image, vk::ImageLayout::eTransferDstOptimal, region);
        }
    }


    vk::UniqueCommandBuffer Model::createCommandBuffer()
    {
        vk::UniqueCommandBuffer cmdBuf = std::move(device.allocateCommandBuffersUnique({ cmdPool, vk::CommandBufferLevel::ePrimary, 1 }).front());
        cmdBuf->begin(vk::CommandBufferBeginInfo{ vk::CommandBufferUsageFlagBits::eOneTimeSubmit });
        return cmdBuf;
    }


    void Model::submitCommandBuffer(vk::CommandBuffer cmdBuf)
    {
        cmdBuf.end();
        vk::UniqueFence fence = device.createFenceUnique({});
        vk::SubmitInfo submitInfo{};
        submitInfo.setCommandBuffers(cmdBuf);
        queue.submit(submitInfo, fence.get());
        auto res = device.waitForFences(fence.get(), true, std::numeric_limits<uint64_t>::max());
        assert(res == vk::Result::eSuccess);
    }


    void Model::createVertexBuffer(Mesh& mesh, bool raytrace)
    {
        using vkbu = vk::BufferUsageFlagBits;
        using vkmp = vk::MemoryPropertyFlagBits;

        vk::BufferUsageFlags bufferUsage;
        if (raytrace) {
            bufferUsage = {
                vkbu::eAccelerationStructureBuildInputReadOnlyKHR
                | vkbu::eStorageBuffer
                | vkbu::eShaderDeviceAddress
                | vkbu::eTransferDst
            };
        } else {
            bufferUsage = {
                vkbu::eVertexBuffer
                | vkbu::eTransferDst
            };
        }
        vk::MemoryPropertyFlags memoryProperty{
            vkmp::eDeviceLocal
        };

        // Create buffer
        mesh.vertexDeviceSize = mesh.vertices.size() * sizeof(Vertex);
        mesh.vertexBuffer = device.createBufferUnique({ {}, mesh.vertexDeviceSize, bufferUsage });

        // Find memory requirements
        const auto requirements = device.getBufferMemoryRequirements(*mesh.vertexBuffer);

        // Allocate memory
        vk::MemoryAllocateFlagsInfo flagsInfo{ vk::MemoryAllocateFlagBits::eDeviceAddress };
        vk::MemoryAllocateInfo allocInfo{};
        allocInfo.allocationSize = requirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(physicalDevice, requirements.memoryTypeBits, memoryProperty);
        allocInfo.pNext = &flagsInfo;
        mesh.vertexDeviceMemory = device.allocateMemoryUnique(allocInfo);

        // Bind memory to buffer
        device.bindBufferMemory(*mesh.vertexBuffer, *mesh.vertexDeviceMemory, 0);

        // Copy from staging buffer
        StagingBuffer stageBuf{ device, physicalDevice, mesh.vertexDeviceSize, bufferUsage, mesh.vertices.data() };
        vk::BufferCopy region{ 0, 0, mesh.vertexDeviceSize };

        vk::UniqueCommandBuffer cmdBuf = createCommandBuffer();
        cmdBuf->copyBuffer(*stageBuf.buffer, *mesh.vertexBuffer, region);
        submitCommandBuffer(*cmdBuf);
    }

    void Model::createIndexBuffer(Mesh& mesh, bool raytrace)
    {
        using vkbu = vk::BufferUsageFlagBits;
        using vkmp = vk::MemoryPropertyFlagBits;

        vk::BufferUsageFlags bufferUsage;
        if (raytrace) {
            bufferUsage = {
                vkbu::eAccelerationStructureBuildInputReadOnlyKHR
                | vkbu::eStorageBuffer
                | vkbu::eShaderDeviceAddress
                | vkbu::eTransferDst
            };
        } else {
            bufferUsage = {
                vkbu::eIndexBuffer
                | vkbu::eTransferDst
            };
        }
        vk::MemoryPropertyFlags memoryProperty{
            vkmp::eDeviceLocal
        };

        // Create buffer
        mesh.vertexDeviceSize = mesh.indices.size() * sizeof(uint32_t);
        mesh.indexBuffer = device.createBufferUnique({ {}, mesh.vertexDeviceSize, bufferUsage });

        // Find memory requirements
        const auto requirements = device.getBufferMemoryRequirements(*mesh.indexBuffer);

        // Allocate memory
        vk::MemoryAllocateFlagsInfo flagsInfo{ vk::MemoryAllocateFlagBits::eDeviceAddress };
        vk::MemoryAllocateInfo allocInfo{};
        allocInfo.allocationSize = requirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(physicalDevice, requirements.memoryTypeBits, memoryProperty);
        allocInfo.pNext = &flagsInfo;
        mesh.indexDeviceMemory = device.allocateMemoryUnique(allocInfo);

        // Bind memory to buffer
        device.bindBufferMemory(*mesh.indexBuffer, *mesh.indexDeviceMemory, 0);

        StagingBuffer stageBuf{ device, physicalDevice, mesh.vertexDeviceSize, bufferUsage, mesh.indices.data() };

        // Copy from staging buffer
        vk::BufferCopy region{ 0, 0, mesh.vertexDeviceSize };
        vk::UniqueCommandBuffer cmdBuf = createCommandBuffer();
        cmdBuf->copyBuffer(*stageBuf.buffer, *mesh.indexBuffer, region);
        submitCommandBuffer(*cmdBuf);
    }

    void Model::loadFromFile(vk::Device device, vk::PhysicalDevice physicalDevice, vk::CommandPool cmdPool, vk::Queue queue, const std::string& filepath)
    {
        this->device = device;
        this->physicalDevice = physicalDevice;
        this->cmdPool = cmdPool;
        this->queue = queue;

        tinygltf::TinyGLTF gltfLoader;
        tinygltf::Model gltfModel;

        std::string err, warn;
        bool result = gltfLoader.LoadASCIIFromFile(&gltfModel, &err, &warn, filepath);
        if (!result) {
            throw std::runtime_error("failed to load gltf file.");
        }
        if (!err.empty()) {
            throw std::runtime_error("gltf error:" + err);
        }
        if (!warn.empty()) {
            throw std::runtime_error("gltf warning:" + warn);
        }

        // Load components
        //loadScenes(gltfModel);
        //loadNodes(gltfModel);
        loadMeshes(gltfModel);
        //loadMaterials(gltfModel);
        //loadTextures(gltfModel);
    }


    void Model::loadNodes(tinygltf::Model& gltfModel)
    {
        for (auto& node : gltfModel.nodes) {
            Node nd;
            nd.children = node.children;
            nd.mesh = node.mesh;
        }
    }

    void Model::loadMeshes(tinygltf::Model& gltfModel)
    {
        for (int index = 0; index < gltfModel.meshes.size(); index++) {
            std::vector<Vertex> vertices;
            std::vector<uint32_t> indices;

            auto& gltfMesh = gltfModel.meshes.at(index);
            auto& gltfPrimitive = gltfMesh.primitives.at(0);

            // Vertex attributes
            auto& attributes = gltfPrimitive.attributes;
            const float* pos = nullptr;
            const float* normal = nullptr;
            const float* uv = nullptr;
            const float* color = nullptr;
            const uint16_t* joint0 = nullptr;
            const float* weight0 = nullptr;
            const float* tangent = nullptr;
            uint32_t numColorComponents;

            assert(attributes.find("POSITION") != attributes.end());

            auto& accessor = gltfModel.accessors[attributes.find("POSITION")->second];
            auto& bufferView = gltfModel.bufferViews[accessor.bufferView];
            auto& buffer = gltfModel.buffers[bufferView.buffer];
            pos = reinterpret_cast<const float*>(&(buffer.data[accessor.byteOffset + bufferView.byteOffset]));

            size_t verticesCount = accessor.count;

            if (attributes.find("NORMAL") != attributes.end()) {
                accessor = gltfModel.accessors[attributes.find("NORMAL")->second];
                bufferView = gltfModel.bufferViews[accessor.bufferView];
                buffer = gltfModel.buffers[bufferView.buffer];
                normal = reinterpret_cast<const float*>(&(buffer.data[accessor.byteOffset + bufferView.byteOffset]));
            }
            if (attributes.find("TEXCOORD_0") != attributes.end()) {
                accessor = gltfModel.accessors[attributes.find("TEXCOORD_0")->second];
                bufferView = gltfModel.bufferViews[accessor.bufferView];
                buffer = gltfModel.buffers[bufferView.buffer];
                uv = reinterpret_cast<const float*>(&(buffer.data[accessor.byteOffset + bufferView.byteOffset]));
            }
            if (attributes.find("COLOR_0") != attributes.end()) {
                accessor = gltfModel.accessors[attributes.find("COLOR_0")->second];
                bufferView = gltfModel.bufferViews[accessor.bufferView];
                buffer = gltfModel.buffers[bufferView.buffer];
                color = reinterpret_cast<const float*>(&(buffer.data[accessor.byteOffset + bufferView.byteOffset]));

                numColorComponents = accessor.type == TINYGLTF_PARAMETER_TYPE_FLOAT_VEC3 ? 3 : 4;
            }
            if (attributes.find("TANGENT") != attributes.end()) {
                accessor = gltfModel.accessors[attributes.find("TANGENT")->second];
                bufferView = gltfModel.bufferViews[accessor.bufferView];
                buffer = gltfModel.buffers[bufferView.buffer];
                tangent = reinterpret_cast<const float*>(&(buffer.data[accessor.byteOffset + bufferView.byteOffset]));
            }
            if (attributes.find("JOINTS_0") != attributes.end()) {
                accessor = gltfModel.accessors[attributes.find("JOINTS_0")->second];
                bufferView = gltfModel.bufferViews[accessor.bufferView];
                buffer = gltfModel.buffers[bufferView.buffer];
                joint0 = reinterpret_cast<const uint16_t*>(&(buffer.data[accessor.byteOffset + bufferView.byteOffset]));
            }
            if (attributes.find("WEIGHTS_0") != attributes.end()) {
                accessor = gltfModel.accessors[attributes.find("WEIGHTS_0")->second];
                bufferView = gltfModel.bufferViews[accessor.bufferView];
                buffer = gltfModel.buffers[bufferView.buffer];
                weight0 = reinterpret_cast<const float*>(&(buffer.data[accessor.byteOffset + bufferView.byteOffset]));
            }

            bool hasSkin = (joint0 && weight0);

            // Pack data to vertex array
            for (size_t v = 0; v < verticesCount; v++) {
                Vertex vert{};
                vert.pos = glm::make_vec3(&pos[v * 3]);
                vert.normal = glm::normalize(glm::vec3(normal ? glm::make_vec3(&normal[v * 3]) : glm::vec3(0.0f)));
                vert.uv = uv ? glm::make_vec2(&uv[v * 2]) : glm::vec2(0.0f);
                vert.joint0 = hasSkin ? glm::vec4(glm::make_vec4(&joint0[v * 4])) : glm::vec4(0.0f);
                if (color) {
                    if (numColorComponents == 3)
                        vert.color = glm::vec4(glm::make_vec3(&color[v * 3]), 1.0f);
                    if (numColorComponents == 4)
                        vert.color = glm::make_vec4(&color[v * 4]);
                } else {
                    vert.color = glm::vec4(1.0f);
                }
                vert.tangent = tangent ? glm::vec4(glm::make_vec4(&tangent[v * 4])) : glm::vec4(0.0f);
                vert.joint0 = hasSkin ? glm::vec4(glm::make_vec4(&joint0[v * 4])) : glm::vec4(0.0f);
                vert.weight0 = hasSkin ? glm::make_vec4(&weight0[v * 4]) : glm::vec4(0.0f);

                vertices.push_back(vert);
            }

            // Get indices
            accessor = gltfModel.accessors[gltfPrimitive.indices];
            bufferView = gltfModel.bufferViews[accessor.bufferView];
            buffer = gltfModel.buffers[bufferView.buffer];

            size_t indicesCount = accessor.count;
            switch (accessor.componentType) {
                case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT:
                {
                    uint32_t* buf = new uint32_t[indicesCount];
                    size_t size = indicesCount * sizeof(uint32_t);
                    memcpy(buf, &buffer.data[accessor.byteOffset + bufferView.byteOffset], size);
                    for (size_t i = 0; i < indicesCount; i++) {
                        indices.push_back(buf[i]);
                    }
                    break;
                }
                case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT:
                {
                    uint16_t* buf = new uint16_t[indicesCount];
                    size_t size = indicesCount * sizeof(uint16_t);
                    memcpy(buf, &buffer.data[accessor.byteOffset + bufferView.byteOffset], size);
                    for (size_t i = 0; i < indicesCount; i++) {
                        indices.push_back(buf[i]);
                    }
                    break;
                }
                case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE:
                {
                    uint8_t* buf = new uint8_t[indicesCount];
                    size_t size = indicesCount * sizeof(uint8_t);
                    memcpy(buf, &buffer.data[accessor.byteOffset + bufferView.byteOffset], size);
                    for (size_t i = 0; i < indicesCount; i++) {
                        indices.push_back(buf[i]);
                    }
                    break;
                }
                default:
                    std::cerr << "Index component type " << accessor.componentType << " not supported!" << std::endl;
                    return;
            }

            Mesh mesh;
            mesh.vertices = std::move(vertices);
            mesh.indices = std::move(indices);
            createVertexBuffer(mesh, true);
            createIndexBuffer(mesh, true);
            mesh.material = gltfPrimitive.material;

            meshes.push_back(std::move(mesh));
        }
    }

    void Model::loadMaterials(tinygltf::Model& gltfModel)
    {
        for (auto& mat : gltfModel.materials) {
            Material material;

            // Base color
            if (mat.values.find("baseColorTexture") != mat.values.end()) {
                material.baseColorTexture = mat.values["baseColorTexture"].TextureIndex();
            }
            if (mat.values.find("baseColorFactor") != mat.values.end()) {
                material.baseColorFactor = glm::make_vec4(mat.values["baseColorFactor"].ColorFactor().data());
            }

            // Metallic / Roughness
            if (mat.values.find("metallicRoughnessTexture") != mat.values.end()) {
                material.metallicRoughnessTexture = mat.values["metallicRoughnessTexture"].TextureIndex();
            }
            if (mat.values.find("roughnessFactor") != mat.values.end()) {
                material.roughnessFactor = static_cast<float>(mat.values["roughnessFactor"].Factor());
            }
            if (mat.values.find("metallicFactor") != mat.values.end()) {
                material.metallicFactor = static_cast<float>(mat.values["metallicFactor"].Factor());
            }

            // Normal
            if (mat.additionalValues.find("normalTexture") != mat.additionalValues.end()) {
                material.normalTexture = mat.additionalValues["normalTexture"].TextureIndex();
            }

            // Emissive
            if (mat.additionalValues.find("emissiveTexture") != mat.additionalValues.end()) {
                material.emissiveTexture = mat.additionalValues["emissiveTexture"].TextureIndex();
            }

            // Occlusion
            if (mat.additionalValues.find("occlusionTexture") != mat.additionalValues.end()) {
                material.occlusionTexture = mat.additionalValues["occlusionTexture"].TextureIndex();
            }

            // Alpha
            if (mat.additionalValues.find("alphaMode") != mat.additionalValues.end()) {
                auto param = mat.additionalValues["alphaMode"];
                if (param.string_value == "BLEND") {
                    material.alphaMode = AlphaMode::Blend;
                }
                if (param.string_value == "MASK") {
                    material.alphaMode = AlphaMode::Mask;
                }
            }
            if (mat.additionalValues.find("alphaCutoff") != mat.additionalValues.end()) {
                material.alphaCutoff = static_cast<float>(mat.additionalValues["alphaCutoff"].Factor());
            }

            materials.push_back(material);
        }
    }

    void Model::loadTextures(tinygltf::Model& gltfModel)
    {
        for (auto& image : gltfModel.images) {
            Texture tex;

            if (image.component == 3) {
                throw std::runtime_error("3 component image is not supported"); // TODO support RGB
            }

            auto buffer = &image.image[0];
            tex.deviceSize = image.image.size();

            // Create image
            tex.extent = vk::Extent2D{ static_cast<uint32_t>(image.width), static_cast<uint32_t>(image.height) };
            tex.format = vk::Format{ vk::Format::eR8G8B8A8Unorm };
            tex.mipLevels = 1; // TODO support mipmap
            createImage(device, physicalDevice, tex);

            // Set image layout
            vk::UniqueCommandBuffer cmdBuf = createCommandBuffer();
            setImageLayout(*cmdBuf, *tex.image, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
            tex.imageLayout = vk::ImageLayout::eTransferDstOptimal;

            // Copy from staging buffer
            StagingBuffer stageBuf{ device, physicalDevice, tex.deviceSize, vk::BufferUsageFlagBits::eTransferSrc, buffer };
            copyBufferToTexture(*cmdBuf, tex, *stageBuf.buffer);
            submitCommandBuffer(*cmdBuf);

            // Create sampler
            vk::SamplerCreateInfo samplerInfo{};
            samplerInfo.magFilter = vk::Filter::eLinear;
            samplerInfo.minFilter = vk::Filter::eLinear;
            samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
            samplerInfo.addressModeU = vk::SamplerAddressMode::eMirroredRepeat;
            samplerInfo.addressModeV = vk::SamplerAddressMode::eMirroredRepeat;
            samplerInfo.addressModeW = vk::SamplerAddressMode::eMirroredRepeat;
            samplerInfo.compareOp = vk::CompareOp::eNever;
            samplerInfo.borderColor = vk::BorderColor::eFloatOpaqueWhite;
            samplerInfo.maxAnisotropy = 1.0;
            samplerInfo.anisotropyEnable = false;
            samplerInfo.maxLod = (float)tex.mipLevels;
            tex.sampler = device.createSamplerUnique(samplerInfo);
        }
    }

}
