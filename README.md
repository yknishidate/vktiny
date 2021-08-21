# vktiny

tiny vulkan wrapper

## features

- Full C++ API
- Vulkan Ray Tracing Pipeline
- glTF loader

## build

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

## examples

![vktiny](https://user-images.githubusercontent.com/30839669/130312274-08636bb7-e392-4954-aea1-9042d292457b.png)


## usage

Adding Resources

```cpp
// Create vertices
std::vector<vkt::Vertex> vertices{
    { { 0.0, -0.3, 0.0} },
    { { 0.3,  0.3, 0.0} },
    { {-0.3,  0.3, 0.0} } };

// Create vertex buffer
vkt::Buffer vertexBuffer;
vertexBuffer.initialize(context, sizeof(Vertex) * vertices.size(),
                        vkBU::eStorageBuffer | vkBU::eShaderDeviceAddress,
                        vkMP::eHostVisible | vkMP::eHostCoherent,
                        vertices.data());

// Add vertex buffer descriptor
vkt::DescriptorManager descManager;
descManager.initialize(context);
descManager.addStorageBuffer(vertexBuffer, /*binding = */0);
```

Pipeline creation

```cpp
vkt::RayTracingPipeline rtPipeline;
rtPipeline.initialize(context);
rtPipeline.addRaygenShader("shader/spv/raygen.rgen.spv");
rtPipeline.addMissShader("shader/spv/miss.rmiss.spv");
rtPipeline.addChitShader("shader/spv/closesthit.rchit.spv");
rtPipeline.prepare(descManager);
```

Loading gltf

```cpp
// Load scene
vkt::Scene scene;
scene.setMeshUsage(vkBU::eAccelerationStructureBuildInputReadOnlyKHR);
scene.setMeshProperties(vkMP::eHostVisible | vkMP::eHostCoherent);
scene.loadFile(context, "asset/Duck/Duck.gltf");
```

Setting up input callbacks

```cpp
context.getInput().setOnMouseButton(
    [&](const int button, const int action, const int mods) {
        vkt::log::info("{} {} {}", button, action, mods);
    }
);
```
