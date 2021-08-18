#pragma once
#include <vector>
#include "vktiny/Scene/Model.hpp"
#include "vktiny/Scene/Mesh.hpp"
#include "vktiny/Scene/Material.hpp"
#include "vktiny/Vulkan/Image.hpp"

namespace tinygltf
{
    class Model;
}

namespace vkt
{
    class Scene
    {
    public:
        Scene() = default;

        void loadFile(const Context& context, const std::string& filepath);

        void setMeshUsage(vk::BufferUsageFlags meshUsage)
        {
            this->meshUsage = meshUsage;
        }

        void setMeshProperties(vk::MemoryPropertyFlags meshProps)
        {
            this->meshProps = meshProps;
        }

        const auto& getMeshes() const { return meshes; }
        const auto& getMaterials() const { return materials; }
        const auto& getTextures() const { return textures; }

    private:
        void loadMeshes(tinygltf::Model& gltfModel);
        void loadMaterials(tinygltf::Model& gltfModel);
        void loadTextures(tinygltf::Model& gltfModel);

        const Context* context;
        vk::BufferUsageFlags meshUsage;
        vk::MemoryPropertyFlags meshProps;

        std::vector<Model> models;
        std::vector<Mesh> meshes;
        std::vector<Material> materials;
        std::vector<Image> textures;
    };
}
