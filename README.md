# [WIP] quick-vkray

A header only library to use Vulkan Ray Tracing quickly.

## Features

-   Support Vulkan Ray Tracing Final Specification
-   Use `vulkan.hpp` (Vulkan C++ binding)
-   Less reliance on third-party libraries
-   Header only
-   Include glTF loader

## Examples

BLAS and TLAS Creation

```cpp
std::vector<vkr::Vertex> vertices{
    { {1.0f, 1.0f, 0.0f} },
    { {-1.0f, 1.0f, 0.0f} },
    { {0.0f, -1.0f, 0.0f} } };

std::vector<uint32_t> indices { 0, 1, 2 };

vkr::Mesh mesh{ device, vertices, indices };

vkr::BottomLevelAccelerationStructure blas{ device, mesh };

vkr::AccelerationStructureInstance asInstance{ 0, glm::mat4(1) };
vkr::TopLevelAccelerationStructure tlas{ device, blas, asInstance };
```

<br>

Vulkan Setup

```cpp
vkr::Window    window    { "vkray", 800, 600 };
vkr::Instance  instance  { window, true };
vkr::Device    device    { instance };
vkr::SwapChain swapChain { device, window };
```

<br>

Shader loading

```cpp
vkr::ShaderManager shaderManager{ device };

shaderManager.addShader("raygen.rgen.spv",
    vk::ShaderStageFlagBits::eRaygenKHR, "main",
    vk::RayTracingShaderGroupTypeKHR::eGeneral);
```

<br>

glTF loading

```cpp
vkr::Model model;
model.loadFromFile(device, "Sponza/Sponza.gltf");
```

## Requirements

Environment

-   Vulkan SDK 1.2.162.0
-   GPU / Driver that support Vulkan Ray Tracing Final Spec
-   C++14 or later

Libraries

-   vulkan.hpp (included in the SDK)
-   GLFW
-   GLM
-   tinygltf (inclued in this repository)
    -   jsonhpp (inclued in this repository)
    -   stb_image (inclued in this repository)

## References

-   [NVIDIA Vulkan Ray Tracing Tutorial](https://nvpro-samples.github.io/vk_raytracing_tutorial_KHR/)
-   [Vulkan-Hpp](https://github.com/KhronosGroup/Vulkan-Hpp)
-   [Vulkan Tutorial](https://vulkan-tutorial.com/)
-   [Vulkan-Samples](https://github.com/KhronosGroup/Vulkan-Samples)
-   [rtxON](https://github.com/iOrange/rtxON)
-   [RayTracingInVulkan](https://github.com/GPSnoopy/RayTracingInVulkan)
-   [SaschaWillems/Vulkan](https://github.com/SaschaWillems/Vulkan)
