#pragma once
#include <vector>
#include "vktiny/Scene/Model.hpp"
#include "vktiny/Scene/Mesh.hpp"
#include "vktiny/Scene/Material.hpp"
#include "vktiny/Vulkan/Image.hpp"
#include "tiny_gltf.h"

namespace vkt
{
    class Scene
    {
    public:
        void loadFile(const Context& context, const std::string& filepath);

    private:
        void loadMeshes(tinygltf::Model& gltfModel);

        const Context* context;
        vk::BufferUsageFlags meshUsage;
        vk::MemoryPropertyFlags meshProps;

        std::vector<Model> models;
        std::vector<Mesh> meshes;
        std::vector<Material> materials;
        std::vector<Image> textures;
    };
}
