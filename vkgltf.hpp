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

// This file does not use any extra wrapper classes other than glTF components.

namespace vkgltf
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
        int32_t baseColorTexture{ -1 };
        glm::vec4 baseColorFactor{ 1.0f };

        // Metallic / Roughness
        int32_t metallicRoughnessTexture{ -1 };
        float metallicFactor{ 1.0f };
        float roughnessFactor{ 1.0f };

        int32_t normalTexture{ -1 };

        int32_t occlusionTexture{ -1 };

        // Emissive
        int32_t emissiveTexture{ -1 };
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

        // Material
        int32_t material{ -1 };
    };


    struct Node
    {
        std::vector<int32_t> children;

        int32_t mesh{ -1 };

        glm::mat4 worldMatrix{ 1.0f };

        glm::vec3 translation{ 1.0f };

        glm::quat rotation{ 1.0f, 0.0f, 0.0f, 0.0f };

        glm::vec3 scale{ 1.0f, 1.0f, 1.0f };
    };


    struct Scene
    {
        std::vector<int32_t> nodes;
    };


    class Model final
    {
    public:

        Model() = default;
        ~Model() = default;
        Model(const Model&) = default;
        Model(Model&&) noexcept = default;
        Model& operator=(const Model&) = default;
        Model& operator=(Model&&) noexcept = default;

        void loadFromFile(vk::Device device, vk::PhysicalDevice physicalDevice, vk::CommandPool cmdPool, const std::string& filepath);

        const std::vector<Scene>& getScenes() const { return scenes; }

        const std::vector<Node>& getNodes() const { return nodes; }

        const std::vector<Mesh>& getMeshes() const { return meshes; }

        const std::vector<Material>& getMaterials() const { return materials; }

        const std::vector<Texture>& getTextures() const { return textures; }

    private:

        void loadScenes(tinygltf::Model& gltfModel);

        void loadNodes(tinygltf::Model& gltfModel);

        void loadMeshes(tinygltf::Model& gltfModel);

        void loadMaterials(tinygltf::Model& gltfModel);

        void loadTextures(tinygltf::Model& gltfModel);

        //vk::Device device;

        //vk::PhysicalDevice physicalDevice;

        std::vector<Scene> scenes;

        std::vector<Node> nodes;

        std::vector<Mesh> meshes;

        std::vector<Material> materials;

        std::vector<Texture> textures;
    };
}


namespace vkgltf
{
    // utils //

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
            //vk::MemoryAllocateFlagsInfo flagsInfo{ vk::MemoryAllocateFlagBits::eDeviceAddress };
            //allocInfo.pNext = &flagsInfo;
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


    void createVertexBuffer(vk::Device device, vk::PhysicalDevice physicalDevice, vk::CommandBuffer cmdBuf, Mesh& mesh)
    {
        using vkbu = vk::BufferUsageFlagBits;
        using vkmp = vk::MemoryPropertyFlagBits;

        auto bufferSize = mesh.vertices.size() * sizeof(Vertex);

        vk::BufferUsageFlags bufferUsage{
            vkbu::eAccelerationStructureBuildInputReadOnlyKHR
            | vkbu::eStorageBuffer
            | vkbu::eShaderDeviceAddress
            | vkbu::eTransferDst
            | vkbu::eVertexBuffer
        };
        vk::MemoryPropertyFlags memoryProperty{
            vkmp::eDeviceLocal
        };

        // Create buffer
        mesh.vertexBuffer = device.createBufferUnique({ {}, bufferSize, bufferUsage });

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

        StagingBuffer stageBuf{ device, physicalDevice, bufferSize, bufferUsage, mesh.vertices.data() };

        // Copy from staging buffer
        vk::BufferCopy region{ 0, 0, bufferSize };
        cmdBuf.copyBuffer(*stageBuf.buffer, *mesh.vertexBuffer, region);
    }


    // Model //
    void Model::loadFromFile(vk::Device device, vk::PhysicalDevice physicalDevice, vk::CommandPool cmdPool, const std::string& filepath)
    {
        //this->device = device;
        //this->physicalDevice = physicalDevice;

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
        loadScenes(gltfModel);
        loadNodes(gltfModel);
        loadMeshes(gltfModel);
        loadMaterials(gltfModel);
        loadTextures(gltfModel);
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


        }
    }


}
