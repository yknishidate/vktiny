#include "vktiny/Scene/Scene.hpp"

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
}

void vkt::Scene::loadMeshes(tinygltf::Model& gltfModel)
{
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
        }
    }
}
