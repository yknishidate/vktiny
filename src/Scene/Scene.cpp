#include "vktiny/Scene/Scene.hpp"

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_NOEXCEPTION
#include "tiny_gltf.h"

void vkt::Scene::loadFile(const Context& context, const std::string& filepath)
{
    this->context = &context;

    tinygltf::TinyGLTF gltfLoader;
    tinygltf::Model gltfModel;

    std::string err, warn;
    bool result;
    if (filepath.substr(filepath.find_last_of(".") + 1) == "gltf") {
        result = gltfLoader.LoadASCIIFromFile(&gltfModel, &err, &warn, filepath);
    } else if (filepath.substr(filepath.find_last_of(".") + 1) == "glb") {
        result = gltfLoader.LoadBinaryFromFile(&gltfModel, &err, &warn, filepath);
    }
    if (!result) {
        throw std::runtime_error("failed to load gltf file.");
    }
    if (!err.empty()) {
        throw std::runtime_error("gltf error:" + err);
    }
    if (!warn.empty()) {
        throw std::runtime_error("gltf warning:" + warn);
    }

    loadMeshes(gltfModel);
    loadMaterials(gltfModel);
    loadTextures(gltfModel);
}

void vkt::Scene::loadMeshes(tinygltf::Model& gltfModel)
{
    //TODO: reserve meshes
    for (int gltfMeshIndex = 0; gltfMeshIndex < gltfModel.meshes.size(); gltfMeshIndex++) {
        auto& gltfMesh = gltfModel.meshes[gltfMeshIndex];
        for (const auto& gltfPrimitive : gltfMesh.primitives) {
            std::vector<Vertex> vertices;
            std::vector<Index> indices;

            // Vertex attributes
            auto& attributes = gltfPrimitive.attributes;
            const float* pos = nullptr;
            const float* normal = nullptr;
            const float* uv = nullptr;
            const float* color = nullptr;
            const float* tangent = nullptr;
            uint32_t numColorComponents;

            assert(attributes.find("POSITION") != attributes.end());

            // Load positions
            auto& accessor = gltfModel.accessors[attributes.find("POSITION")->second];
            auto& bufferView = gltfModel.bufferViews[accessor.bufferView];
            auto& buffer = gltfModel.buffers[bufferView.buffer];
            pos = reinterpret_cast<const float*>(&(buffer.data[accessor.byteOffset + bufferView.byteOffset]));
            size_t verticesCount = accessor.count;

            // Pack data to vertex array
            for (size_t v = 0; v < verticesCount; v++) {
                Vertex vert;
                vert.pos = glm::make_vec3(&pos[v * 3]);
                vertices.push_back(vert);
            }

            // Get indices
            {
                auto& accessor = gltfModel.accessors[gltfPrimitive.indices];
                auto& bufferView = gltfModel.bufferViews[accessor.bufferView];
                auto& buffer = gltfModel.buffers[bufferView.buffer];

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
                    throw std::runtime_error("Index component type not supported");
                    return;
                }
            }

            meshes.emplace_back();
            meshes.back().initialize(*context, vertices, indices, meshUsage, meshProps);
            meshes.back().setMaterialIndex(gltfPrimitive.material);
        }
    }
}

void vkt::Scene::loadMaterials(tinygltf::Model& gltfModel)
{
    materials.reserve(gltfModel.materials.size());
    for (auto& mat : gltfModel.materials) {
        Material material;

        // Base color
        if (mat.values.find("baseColorTexture") != mat.values.end()) {
            material.baseColorTextureIndex = mat.values["baseColorTexture"].TextureIndex();
        }
        if (mat.values.find("baseColorFactor") != mat.values.end()) {
            material.baseColorFactor = glm::make_vec4(mat.values["baseColorFactor"].ColorFactor().data());
        }

        // Metallic / Roughness
        if (mat.values.find("metallicRoughnessTexture") != mat.values.end()) {
            material.metallicRoughnessTextureIndex = mat.values["metallicRoughnessTexture"].TextureIndex();
        }
        if (mat.values.find("roughnessFactor") != mat.values.end()) {
            material.roughnessFactor = static_cast<float>(mat.values["roughnessFactor"].Factor());
        }
        if (mat.values.find("metallicFactor") != mat.values.end()) {
            material.metallicFactor = static_cast<float>(mat.values["metallicFactor"].Factor());
        }

        // Normal
        if (mat.additionalValues.find("normalTexture") != mat.additionalValues.end()) {
            material.normalTextureIndex = mat.additionalValues["normalTexture"].TextureIndex();
        }

        // Emissive
        if (mat.additionalValues.find("emissiveTexture") != mat.additionalValues.end()) {
            material.emissiveTextureIndex = mat.additionalValues["emissiveTexture"].TextureIndex();
        }

        // Occlusion
        if (mat.additionalValues.find("occlusionTexture") != mat.additionalValues.end()) {
            material.occlusionTextureIndex = mat.additionalValues["occlusionTexture"].TextureIndex();
        }

        materials.push_back(material);
    }
}

void vkt::Scene::loadTextures(tinygltf::Model& gltfModel)
{
    textures.reserve(gltfModel.images.size());
    for (auto& image : gltfModel.images) {
        Image tex;

        if (image.component == 3) {
            throw std::runtime_error("3 component image is not supported"); // TODO support RGB
        }

        auto imageData = &image.image[0];
        uint32_t size = image.image.size();

        // Create image
        // TODO: set image aspect
        vk::Extent2D extent{ static_cast<uint32_t>(image.width), static_cast<uint32_t>(image.height) };
        tex.initialize(*context, extent, vk::Format::eR8G8B8A8Unorm,
                       vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst);
        tex.createImageView();
        tex.transitionLayout(vk::ImageLayout::eTransferDstOptimal);

        // Copy from staging buffer
        Buffer stageBuffer;
        stageBuffer.initialize(*context, size, vk::BufferUsageFlagBits::eTransferSrc,
                               vk::MemoryPropertyFlagBits::eHostVisible |
                               vk::MemoryPropertyFlagBits::eHostCoherent,
                               imageData);
        tex.copyBuffer(stageBuffer);
        tex.transitionLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
        tex.createSampler();
        textures.push_back(std::move(tex));
    }
}
