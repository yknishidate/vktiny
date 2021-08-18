# vktiny

tiny vulkan wrapper

## usage

```
git clone --recursive https://github.com/nishidate-yuki/vktiny.git
cd vktiny
cmake . -Bbuild
.\build\vktiny.sln
```

## dependencies

- [glfw](https://github.com/glfw/glfw.git)
- [glm](https://github.com/g-truc/glm.git)
- [Vulkan-Headers](https://github.com/KhronosGroup/Vulkan-Headers.git)
- [spdlog](https://github.com/gabime/spdlog.git)
- [tinygltf](https://github.com/syoyo/tinygltf.git)

## example

Resource adding

```cpp
// Create vertices
std::vector<Vertex> vertices{
    { { 0.0, -0.3, 0.0} },
    { { 0.3,  0.3, 0.0} },
    { {-0.3,  0.3, 0.0} } };

// Create vertex buffer
Buffer vertexBuffer;
vertexBuffer.initialize(context, sizeof(Vertex) * vertices.size(),
                        vkBU::eStorageBuffer | vkBU::eShaderDeviceAddress,
                        vkMP::eHostVisible | vkMP::eHostCoherent,
                        vertices.data());

// Add vertex buffer descriptor
DescriptorManager descManager;
descManager.initialize(context);
descManager.addStorageBuffer(vertexBuffer, /*binding = */0);
```

Pipeline creation

```cpp
RayTracingPipeline rtPipeline;
rtPipeline.initialize(context);
rtPipeline.addRaygenShader("shader/spv/raygen.rgen.spv");
rtPipeline.addMissShader("shader/spv/miss.rmiss.spv");
rtPipeline.addChitShader("shader/spv/closesthit.rchit.spv");
rtPipeline.prepare(descManager);
```

gltf loading

```cpp
// Load scene
vkt::Scene scene;
scene.setMeshUsage(vkBU::eAccelerationStructureBuildInputReadOnlyKHR);
scene.setMeshProperties(vkMP::eHostVisible | vkMP::eHostCoherent);
scene.loadFile(context, "asset/Duck/Duck.gltf");

// Output scene info
for (auto&& mesh : scene.getMeshes()) {
    spdlog::info("mesh");
    spdlog::info("  vertices: {}", mesh.getVertices().size());
    spdlog::info("  indices: {}", mesh.getIndices().size());
}
for (auto&& mat : scene.getMaterials()) {
    spdlog::info("material");
    spdlog::info("  baseColorFactor: {}", glm::to_string(mat.baseColorFactor));
    spdlog::info("  baseColorTextureIndex: {}", mat.baseColorTextureIndex);
}
```
