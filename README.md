# vktiny [WIP]

Thin Vulkan C++ wrapper on [vulkan.hpp](https://github.com/KhronosGroup/Vulkan-Hpp)

## Build

```
git clone --recursive https://github.com/nishidate-yuki/vktiny.git
cd vktiny
cmake -DVKTINY_EXAMPLES=ON . -Bbuild
.\build\vktiny.sln
```

## Dependencies

- [glfw](https://github.com/glfw/glfw.git)
- [Vulkan-Headers](https://github.com/KhronosGroup/Vulkan-Headers.git)
- [glslang](https://github.com/KhronosGroup/glslang.git)

## Examples

- [x] hello_compute
- [ ] ~~camera~~
- [ ] ~~gltf_loading~~
- [ ] ~~pathtracing~~
- [ ] ~~raytracing_triangle~~
- [ ] ~~sponza~~

## Usage

```cpp
/* Example: Compute pipeline creation */
const std::string shader = R"(
#version 460
layout(local_size_x = 1, local_size_y = 1) in;
layout(binding = 0, rgba8) uniform image2D renderImage;

void main()
{
	imageStore(renderImage, ivec2(gl_GlobalInvocationID.xy), vec4(1));
}
)";

vkt::ComputeShaderModule shaderModule{ context, shader };
vkt::ComputePipeline pipeline{ context, descSetLayout, shaderModule };
```

<!--
![vktiny](https://user-images.githubusercontent.com/30839669/130312423-1ed40a68-d7ad-4512-bf08-63be05bb3444.png)

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
``` -->
