# quick-vkray

A header only library to make Vulkan Ray Tracing easy to use.

## Features

-   Support Vulkan Ray Tracing Final Specification
-   Use `vulkan.hpp` (Vulkan C++ binding)
-   Less reliance on third-party libraries
-   Header only
-   Include glTF loader(**TODO**)

## Examples

BLAS and TLAS Creation

```cpp
#include "vkray.hpp"
std::vector<vkr::Vertex> vertices{
    { {1.0f, 1.0f, 0.0f} },
    { {-1.0f, 1.0f, 0.0f} },
    { {0.0f, -1.0f, 0.0f} } };

std::vector<uint32_t> indices { 0, 1, 2 };

vkr::BottomLevelAccelerationStructure blas{ device, vertices, indices };

vkr::AccelerationStructureInstance asInstance{ 0, glm::mat4(1), 0 };
vkr::TopLevelAccelerationStructure tlas{ device, blas, asInstance };
```

<br>

Vulkan Setup

```cpp
#include "vkray.hpp"
vkr::Window    window    { "vkray", 800, 600 };
vkr::Instance  instance  { window, true };
vkr::Device    device    { instance };
vkr::SwapChain swapChain { device };
```

<br>

Shader loading

```cpp
#include "vkray.hpp"
vkr::ShaderManager shaderManager{ device };

shaderManager.addShader("raygen.rgen.spv",
    vk::ShaderStageFlagBits::eRaygenKHR, "main",
    vk::RayTracingShaderGroupTypeKHR::eGeneral);
```

## Requirements

Environment

-   Vulkan SDK 1.2.162.0
-   GPU / Driver that support Vulkan Ray Tracing Final Spec

Libraries

-   vulkan.hpp (included in the SDK)
-   GLFW
-   GLM

## References

-   [NVIDIA Vulkan Ray Tracing Tutorial](https://nvpro-samples.github.io/vk_raytracing_tutorial_KHR/)
-   [Vulkan-Hpp](https://github.com/KhronosGroup/Vulkan-Hpp)
-   [Vulkan Tutorial](https://vulkan-tutorial.com/)
-   [Vulkan-Samples](https://github.com/KhronosGroup/Vulkan-Samples)
-   [rtxON](https://github.com/iOrange/rtxON)
-   [RayTracingInVulkan](https://github.com/GPSnoopy/RayTracingInVulkan)
-   [SaschaWillems/Vulkan](https://github.com/SaschaWillems/Vulkan)
