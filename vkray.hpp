#pragma once

#include <set>
#include <map>
#include <vector>
#include <string>
#include <fstream>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <algorithm>

#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>
VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

#include <GLFW/glfw3.h>

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

namespace vkr
{
    class Window;

    class Instance;

    class Device;

    class Image;

    class Buffer;

    class DescriptorSets;

    class ShaderManager;

    // Model components
    class Model;
    struct Scene;
    struct Node;
    struct Mesh;
    struct Material;
    struct Texture;


    class Window final
    {
    public:

        Window(const std::string& title, const uint32_t width, const uint32_t height,
               bool cursorDisabled = false, bool fullscreen = false, bool resizable = false);

        ~Window()
        {
            if (window != nullptr) {
                glfwDestroyWindow(window);
            }

            glfwTerminate();
            glfwSetErrorCallback(nullptr);
        }

        // non copyable / non movable
        Window(const Window&) = delete;
        Window(Window&&) = delete;
        Window& operator = (const Window&) = delete;
        Window& operator = (Window&&) = delete;

        std::string getTitle() const { return title; }

        float getContentScale() const
        {
            float xscale, yscale;

            glfwGetWindowContentScale(window, &xscale, &yscale);

            return xscale;
        }

        vk::Extent2D getFramebufferSize() const
        {
            int width, height;

            glfwGetFramebufferSize(window, &width, &height);

            return vk::Extent2D{ static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
        }

        vk::Extent2D getWindowSize() const
        {
            int width, height;

            glfwGetWindowSize(window, &width, &height);

            return vk::Extent2D{ static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
        }

        const char* getKeyName(int key, int scancode) const
        {
            return glfwGetKeyName(key, scancode);
        }

        std::vector<const char*> getRequiredInstanceExtensions() const
        {
            uint32_t glfwExtensionCount = 0;

            const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

            return std::vector<const char*>(glfwExtensions, glfwExtensions + glfwExtensionCount);
        }

        double getTime() const
        {
            return glfwGetTime();
        }

        void close()
        {
            glfwSetWindowShouldClose(window, 1);
        }

        bool isMinimized() const
        {
            const auto size = getFramebufferSize();
            return size.height == 0 && size.width == 0;
        }

        bool shouldClose() const
        {
            return glfwWindowShouldClose(window);
        }

        void pollEvents() const
        {
            glfwPollEvents();
        }

        void waitForEvents() const
        {
            glfwWaitEvents();
        }

        vk::SurfaceKHR createWindowSurface(vk::Instance instance) const
        {
            VkSurfaceKHR surface;

            if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
                throw std::runtime_error("failed to create window surface!");
            }

            return vk::SurfaceKHR(surface);
        }

        std::function<void(int key, int scancode, int action, int mods)> onKey;
        std::function<void(double xpos, double ypos)> onCursorPosition;
        std::function<void(int button, int action, int mods)> onMouseButton;
        std::function<void(double xoffset, double yoffset)> onScroll;

    private:

        GLFWwindow* window;

        std::string title;

        uint32_t width;
        uint32_t height;

        bool cursorDisabled;
        bool fullscreen;
        bool resizable;
    };


    class Instance final
    {
    public:

        Instance(const Window& window, const bool enableValidationLayers);

        // non copyable / non movable
        Instance(const Instance&) = delete;
        Instance(Instance&&) = delete;
        Instance& operator = (const Instance&) = delete;
        Instance& operator = (Instance&&) = delete;

        vk::Instance getHandle() const { return *instance; }

        const std::vector<vk::ExtensionProperties>& getExtensionProperties() const { return extensionProperties; }

        const std::vector<vk::PhysicalDevice>& getPhysicalDevices() const { return physicalDevices; }

        const std::vector<const char*>& getValidationLayers() const { return validationLayers; }

        vk::UniqueSurfaceKHR createSurface() const;

        vk::PhysicalDevice pickSuitablePhysicalDevice() const;

    private:

        void createDebugMessenger();

        static void checkVulkanMinimumVersion(uint32_t minVersion);

        static void checkVulkanValidationLayerSupport(const std::vector<const char*>& validationLayers);

        const Window& window;

        vk::UniqueInstance instance;

        vk::UniqueDebugUtilsMessengerEXT messenger;

        std::vector<const char*> validationLayers;

        std::vector<vk::PhysicalDevice> physicalDevices;

        std::vector<vk::ExtensionProperties> extensionProperties;

        bool enableValidationLayers;
    };


    class Device final
    {
    public:

        explicit Device(const Instance& instance);

        // non copyable / non movable
        Device(const Device&) = delete;
        Device(Device&&) = delete;
        Device& operator = (const Device&) = delete;
        Device& operator = (Device&&) = delete;

        vk::Device getHandle() const { return *device; }

        vk::PhysicalDevice getPhysicalDevice() const { return physicalDevice; }

        vk::SurfaceKHR getSurface() const { return *surface; }

        uint32_t getGraphicsFamilyIndex() const { return graphicsFamilyIndex; }
        uint32_t getComputeFamilyIndex() const { return computeFamilyIndex; }
        uint32_t getPresentFamilyIndex() const { return presentFamilyIndex; }
        uint32_t getTransferFamilyIndex() const { return transferFamilyIndex; }

        vk::Queue getGraphicsQueue() const { return graphicsQueue; }
        vk::Queue getComputeQueue() const { return computeQueue; }
        vk::Queue getPresentQueue() const { return presentQueue; }
        vk::Queue getTransferQueue() const { return transferQueue; }

        vk::CommandPool getCommandPool() const { return *commandPool; }

        void waitIdle() const { device->waitIdle(); }

        // for other objects
        uint32_t findMemoryType(const uint32_t typeFilter, const vk::MemoryPropertyFlags properties) const;

        vk::UniqueCommandBuffer createCommandBuffer(vk::CommandBufferLevel level = vk::CommandBufferLevel::ePrimary, bool begin = true,
                                                    vk::CommandBufferUsageFlags usage = vk::CommandBufferUsageFlagBits::eOneTimeSubmit) const;

        void submitCommandBuffer(vk::CommandBuffer& commandBuffer) const;

        vk::UniquePipeline createRayTracingPipeline(const DescriptorSets& descSets, const ShaderManager& shaderManager, uint32_t maxRecursionDepth);

    private:

        void checkRequiredExtensions(vk::PhysicalDevice physicalDevice) const;

        const std::vector<const char*> requiredExtensions{
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
            VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME,
            VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME,
            VK_KHR_MAINTENANCE3_EXTENSION_NAME,
            VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME,
            VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
            VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
            VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
            VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME
        };

        vk::UniqueDevice device;

        vk::PhysicalDevice physicalDevice;

        vk::UniqueSurfaceKHR surface;

        vk::UniqueCommandPool commandPool;

        uint32_t graphicsFamilyIndex{};
        uint32_t computeFamilyIndex{};
        uint32_t presentFamilyIndex{};
        uint32_t transferFamilyIndex{};

        vk::Queue graphicsQueue{};
        vk::Queue computeQueue{};
        vk::Queue presentQueue{};
        vk::Queue transferQueue{};
    };


    class SwapChain final
    {
    public:

        explicit SwapChain(const Device& device, const Window& window);

        ~SwapChain()
        {
            for (size_t i = 0; i < maxFramesInFlight; i++) {
                device.getHandle().destroyFence(inFlightFences[i]);
            }
        }

        // non copyable / non movable
        SwapChain(const SwapChain&) = delete;
        SwapChain(SwapChain&&) = delete;
        SwapChain& operator = (const SwapChain&) = delete;
        SwapChain& operator = (SwapChain&&) = delete;

        vk::SwapchainKHR getSwapChain() const { return *swapChain; }

        vk::PhysicalDevice getPhysicalDevice() const { return physicalDevice; }

        uint32_t getImageCount() const { return imageCount; }

        const std::vector<vk::Image>& getImages() const { return images; }

        const std::vector<vk::UniqueImageView>& getImageViews() const { return imageViews; }

        const vk::Extent2D& getExtent() const { return extent; }

        vk::Format getFormat() const { return format; }

        vk::PresentModeKHR getPresentMode() const { return presentMode; }

        /// <summary>
        /// Creates an image to store the ray tracing results.
        /// </summary>
        std::unique_ptr<Image> createStorageImage() const;

        void initDrawCommandBuffers(vk::Pipeline pipeline, const DescriptorSets& descSets, const ShaderManager& shaderManager, vkr::Image& storageImage);

        uint32_t acquireNextImage();

        void draw();

    private:

        struct SupportDetails
        {
            vk::SurfaceCapabilitiesKHR capabilities;

            std::vector<vk::SurfaceFormatKHR> formats;

            std::vector<vk::PresentModeKHR> presentModes;
        };

        static SupportDetails querySwapChainSupport(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface);

        static vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& formats);

        static vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& presentModes);

        static vk::Extent2D chooseSwapExtent(const Window& window, const vk::SurfaceCapabilitiesKHR& capabilities);

        static uint32_t chooseImageCount(const vk::SurfaceCapabilitiesKHR& capabilities);

        const vk::PhysicalDevice physicalDevice;

        const Device& device;

        vk::UniqueSwapchainKHR swapChain;

        uint32_t imageCount;

        vk::PresentModeKHR presentMode;

        vk::Format format;

        vk::Extent2D extent;

        std::vector<vk::Image> images;

        std::vector<vk::UniqueImageView> imageViews;

        std::vector<vk::UniqueCommandBuffer> drawCmdBufs;

        size_t currentFrame = 0;

        const int maxFramesInFlight = 2;

        std::vector<vk::UniqueSemaphore> imageAvailableSemaphores;
        std::vector<vk::UniqueSemaphore> renderFinishedSemaphores;

        std::vector<vk::Fence> inFlightFences;
        std::vector<vk::Fence> imagesInFlight;
    };


    class Image
    {
    public:

        /// <summary>
        /// Creates a image handle, but does not allocate memory.
        /// </summary>
        Image(const Device& device, vk::Extent2D extent, vk::Format format,
              vk::ImageTiling tiling = vk::ImageTiling::eOptimal,
              vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst);

        /// <summary>
        /// Creates a image handle and then allocates and binds the memory.
        /// </summary>
        Image(const Device& device, vk::Extent2D extent, vk::Format format,
              vk::MemoryPropertyFlags properties, vk::ImageAspectFlags aspectFlags,
              vk::ImageTiling tiling = vk::ImageTiling::eOptimal,
              vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst);

        // non copyable
        Image(const Image&) = delete;
        Image& operator = (const Image&) = delete;

        vk::Image getHandle() const { return *image; }

        vk::Extent2D getExtent() const { return extent; }

        vk::Format getFormat() const { return format; }

        vk::ImageView getView() const { return *view; }

        vk::ImageLayout getLayout() const { return imageLayout; }

        void setLayout(vk::ImageLayout layout) { imageLayout = layout; }

        vk::DescriptorImageInfo createDescriptorInfo() const;

        void copyFrom(vk::CommandBuffer& cmdBuf, const Buffer& buffer);

        void copyFrom(const Device& device, const Buffer& buffer);

        void transitionImageLayout(vk::CommandBuffer& cmdBuf, vk::ImageLayout newLayout);

    private:

        vk::UniqueImage image;

        vk::UniqueImageView view;

        vk::UniqueDeviceMemory memory;

        vk::Extent2D extent;

        vk::Format format;

        vk::ImageLayout imageLayout;
    };


    class Buffer
    {
    public:

        /// <summary>
        /// Creates a buffer handle, but does not allocate memory.
        /// </summary>
        Buffer(const Device& device, vk::DeviceSize size, vk::BufferUsageFlags usage);

        /// <summary>
        /// Creates a buffer handle and then allocates and binds the memory.
        /// </summary>
        Buffer(const Device& device, vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties);

        /// <summary>
        /// Creates a buffer handle, allocates memory, and then stores the data.
        /// </summary>
        Buffer(const Device& device, vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, void* data);

        // non copyable
        Buffer(const Buffer&) = delete;
        Buffer& operator = (const Buffer&) = delete;

        vk::Buffer getHandle() const { return *buffer; }

        vk::DeviceSize getSize() const { return size; }

        uint64_t getDeviceAddress() const { return deviceAddress; }

        void copyFrom(const Device& device, const Buffer& src);

        vk::DescriptorBufferInfo createDescriptorInfo() const;

        void map();

        void map(vk::DeviceSize size, vk::DeviceSize offset = 0);

        void unmap();

        void copy(void* data);

    private:

        vk::Device device;

        vk::UniqueBuffer buffer;

        vk::UniqueDeviceMemory memory;

        vk::DeviceSize size;

        uint64_t deviceAddress;

        void* mapped = nullptr;
    };


    class DescriptorSetBindings final
    {
    public:

        DescriptorSetBindings() = default;

        // non copyable
        DescriptorSetBindings(const DescriptorSetBindings&) = delete;
        DescriptorSetBindings& operator = (const DescriptorSetBindings&) = delete;

        void addBindging(uint32_t binding, vk::DescriptorType type, uint32_t count,
                         vk::ShaderStageFlags stageFlags, const vk::Sampler* pImmutableSampler = nullptr);

        vk::UniqueDescriptorSetLayout createLayout(const Device& device, vk::DescriptorSetLayoutCreateFlags flags = {}) const;

        void addRequiredPoolSizes(std::vector<vk::DescriptorPoolSize>& poolSizes) const;

        vk::WriteDescriptorSet makeWrite(vk::DescriptorSet dstSet, uint32_t dstBinding,
                                         const vk::DescriptorImageInfo* pImageInfo, uint32_t arrayElement = 0) const;

        vk::WriteDescriptorSet makeWrite(vk::DescriptorSet dstSet, uint32_t dstBinding,
                                         const vk::DescriptorBufferInfo* pBufferInfo, uint32_t arrayElement = 0) const;

        vk::WriteDescriptorSet makeWrite(vk::DescriptorSet dstSet, uint32_t dstBinding,
                                         const vk::WriteDescriptorSetAccelerationStructureKHR* pASInfo, uint32_t arrayElement = 0) const;

    private:

        std::vector<vk::DescriptorSetLayoutBinding> bindings;
    };


    class DescriptorSets final
    {
    public:

        DescriptorSets(const Device& device, uint32_t numSets = 1);

        // non copyable / non movable
        DescriptorSets(const DescriptorSets&) = delete;
        DescriptorSets(DescriptorSets&&) = delete;
        DescriptorSets& operator = (const DescriptorSets&) = delete;
        DescriptorSets& operator = (DescriptorSets&&) = delete;

        vk::PipelineLayout getPipelineLayout() const { return *pipeLayout; }

        std::vector<vk::DescriptorSet> getDescriptorSets() const;

        std::vector<vk::DescriptorSetLayout> getDescriptorSetLayouts() const;

        void addBindging(uint32_t setIndex, uint32_t binding, vk::DescriptorType type, uint32_t count,
                         vk::ShaderStageFlags stageFlags, const vk::Sampler* pImmutableSampler = nullptr);

        /// <summary>
        /// Creates a pipeline layout. It also creates each descriptor set internally.
        /// </summary>
        void initPipelineLayout();

        vk::PipelineLayout createPipelineLayout();

        void addWriteInfo(uint32_t setIndex, uint32_t binding, vk::WriteDescriptorSetAccelerationStructureKHR writeInfo)
        {
            writeDescSets.push_back(bindingsArray[setIndex]->makeWrite(*descSets[setIndex], binding, &writeInfo));
        }

        void addWriteInfo(uint32_t setIndex, uint32_t binding, vk::DescriptorImageInfo writeInfo)
        {
            writeDescSets.push_back(bindingsArray[setIndex]->makeWrite(*descSets[setIndex], binding, &writeInfo));
        }

        void addWriteInfo(uint32_t setIndex, uint32_t binding, vk::DescriptorBufferInfo writeInfo)
        {
            writeDescSets.push_back(bindingsArray[setIndex]->makeWrite(*descSets[setIndex], binding, &writeInfo));
        }

        void allocate();

        void update()
        {
            device.getHandle().updateDescriptorSets(writeDescSets, 0);
            // TODO: Clear Write Descs?
        }

    private:

        const Device& device;

        const uint32_t numSets;

        vk::UniqueDescriptorPool descPool;

        vk::UniquePipelineLayout pipeLayout;

        std::vector<vk::UniqueDescriptorSet> descSets;

        std::vector<vk::UniqueDescriptorSetLayout> descSetLayouts;

        std::vector<std::unique_ptr<DescriptorSetBindings>> bindingsArray;

        std::vector<vk::WriteDescriptorSet> writeDescSets;
    };


    class ShaderManager final
    {
    public:

        ShaderManager(const Device& device) : device(device) {}

        // non copyable / non movable
        ShaderManager(const ShaderManager&) = delete;
        ShaderManager(ShaderManager&&) = delete;
        ShaderManager& operator = (const ShaderManager&) = delete;
        ShaderManager& operator = (ShaderManager&&) = delete;

        auto getStages() const { return stages; }

        auto getRayTracingGroups() const { return rtGroups; }

        auto getRaygenRegion() const { return raygenRegion; }
        auto getMissRegion() const { return missRegion; }
        auto getHitRegion() const { return hitRegion; }

        void addShader(const std::string& filename, vk::ShaderStageFlagBits stage, const char* pName, vk::RayTracingShaderGroupTypeKHR groupType);

        void addShader(uint32_t addedShaderIndex, vk::ShaderStageFlagBits stage, const char* pName, vk::RayTracingShaderGroupTypeKHR groupType);

        void initShaderBindingTable(const vk::Pipeline& pipeline, uint32_t raygenOffset, uint32_t missOffset, uint32_t hitOffset);

    private:

        vk::UniqueShaderModule createShaderModule(const std::string& filename);

        const Device& device;

        std::vector<vk::UniqueShaderModule> modules;

        std::vector<vk::PipelineShaderStageCreateInfo> stages;

        std::vector<vk::RayTracingShaderGroupCreateInfoKHR> rtGroups;

        std::unique_ptr<Buffer> raygenShaderBindingTable;
        std::unique_ptr<Buffer> missShaderBindingTable;
        std::unique_ptr<Buffer> hitShaderBindingTable;

        vk::StridedDeviceAddressRegionKHR raygenRegion;
        vk::StridedDeviceAddressRegionKHR missRegion;
        vk::StridedDeviceAddressRegionKHR hitRegion;
    };


    struct AccelerationStructureInstance
    {
        uint32_t modelIndex;

        glm::mat4 transformMatrix;

        uint32_t textureOffset;
    };


    class AccelerationStructure
    {
    public:

        AccelerationStructure() = default;

        // non copyable
        AccelerationStructure(const AccelerationStructure&) = delete;
        AccelerationStructure& operator = (const AccelerationStructure&) = delete;

        vk::AccelerationStructureKHR getHandle() { return *accelerationStructure; }

        const uint64_t getDeviceAddress() const { return deviceAddress; }

    protected:

        void build(const Device& device, vk::AccelerationStructureGeometryKHR& geometry,
                   const vk::AccelerationStructureTypeKHR& asType, uint32_t primitiveCount);

        void createBuffer(const Device& device, vk::AccelerationStructureBuildSizesInfoKHR buildSizesInfo);

        vk::UniqueAccelerationStructureKHR accelerationStructure;

        std::unique_ptr<Buffer> buffer;

        uint64_t deviceAddress;
    };


    class BottomLevelAccelerationStructure final : public AccelerationStructure
    {
    public:

        BottomLevelAccelerationStructure(const Device& device, const Mesh& mesh);
    };


    class TopLevelAccelerationStructure final : public AccelerationStructure
    {
    public:

        // TODO: vector input
        //TopLevelAccelerationStructure(const Device& device, std::vector<AccelerationStructureInstance>& instances);

        TopLevelAccelerationStructure(const Device& device, BottomLevelAccelerationStructure& blas, AccelerationStructureInstance& instance);

        vk::WriteDescriptorSetAccelerationStructureKHR createWrite() const
        {
            return { 1, &accelerationStructure.get() };
        }
    };


} // vkr

namespace vkr
{
    // Model components
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
        std::unique_ptr<Image> image;

        vk::DeviceSize deviceSize;

        vk::UniqueSampler sampler;

        uint32_t mipLevels;

        uint32_t layerCount;

        vk::DescriptorImageInfo createDescriptorInfo() const
        {
            return { *sampler, image->getView(), image->getLayout() };
        }
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
        int baseColorTexture{ -1 };
        glm::vec4 baseColorFactor{ 1.0f };

        // Metallic / Roughness
        int metallicRoughnessTexture{ -1 };
        float metallicFactor{ 1.0f };
        float roughnessFactor{ 1.0f };

        int normalTexture{ -1 };

        int occlusionTexture{ -1 };

        // Emissive
        int emissiveTexture{ -1 };
        glm::vec3 emissiveFactor{ 0.0f };

        AlphaMode alphaMode{ AlphaMode::Opaque };
        float alphaCutoff{ 0.5f };

        bool doubleSided{ false };
    };


    struct Mesh
    {
        // Vertex
        std::vector<Vertex> vertices;

        std::unique_ptr<Buffer> vertexBuffer;

        // Index
        std::vector<uint32_t> indices;

        std::unique_ptr<Buffer> indexBuffer;

        int material{ -1 };

        void create(const Device& device, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
    };


    struct Node
    {
        std::vector<int> children;

        int mesh{ -1 };

        glm::mat4 worldMatrix{ 1.0f };

        glm::vec3 translation{ 1.0f };

        glm::quat rotation{ 1.0f, 0.0f, 0.0f, 0.0f };

        glm::vec3 scale{ 1.0f, 1.0f, 1.0f };
    };


    struct Scene
    {
        std::vector<int> nodes;
    };


    class Model final
    {
    public:

        Model() = default;
        ~Model() = default;

        // non copyable
        Model(const Model&) = delete;
        Model& operator=(const Model&) = delete;

        const std::vector<Scene>& getScenes() const { return scenes; }

        const std::vector<Node>& getNodes() const { return nodes; }

        const std::vector<Mesh>& getMeshes() const { return meshes; }

        const std::vector<Material>& getMaterials() const { return materials; }

        const std::vector<Texture>& getTextures() const { return textures; }

        void loadFromFile(const Device& device, const std::string& filepath);

        void setFlipY(bool flipY) { this->flipY = flipY; }

    private:

        void loadScenes(tinygltf::Model& gltfModel);

        void loadNodes(tinygltf::Model& gltfModel);

        void loadMeshes(const Device& device, tinygltf::Model& gltfModel);

        void loadMaterials(tinygltf::Model& gltfModel);

        void loadTextures(const Device& device, tinygltf::Model& gltfModel);

        vk::Device device;

        vk::PhysicalDevice physicalDevice;

        vk::CommandPool cmdPool;

        vk::Queue queue;

        std::vector<Scene> scenes;

        std::vector<Node> nodes;
        vk::UniqueBuffer nodesBuffer;
        vk::UniqueDeviceMemory nodesDeviceMemory;

        std::vector<Mesh> meshes;
        vk::UniqueBuffer meshesBuffer;
        vk::UniqueDeviceMemory meshesDeviceMemory;

        std::vector<Material> materials;

        std::vector<Texture> textures;

        bool flipY{ true };
    };

} // vkr



//----------------//
// implementation //
//----------------//

namespace vkr
{
#if defined(_DEBUG)
    VKAPI_ATTR VkBool32 VKAPI_CALL
        debugUtilsMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                                    VkDebugUtilsMessengerCallbackDataEXT const* pCallbackData, void* /*pUserData*/)
    {
        std::cerr << "messageIDName   = " << pCallbackData->pMessageIdName << "\n";

        for (uint8_t i = 0; i < pCallbackData->objectCount; i++) {
            std::cerr << "objectType      = " << vk::to_string(static_cast<vk::ObjectType>(pCallbackData->pObjects[i].objectType)) << "\n";
        }

        std::cerr << pCallbackData->pMessage << "\n\n";

        return VK_FALSE;
    }
#endif

    void glfwErrorCallback(const int error, const char* const description)
    {
        std::cerr << "ERROR: GLFW: " << description << " (code: " << error << ")" << std::endl;
    }

    void glfwKeyCallback(GLFWwindow* window, const int key, const int scancode, const int action, const int mods)
    {
        auto* const this_ = static_cast<Window*>(glfwGetWindowUserPointer(window));
        if (this_->onKey) {
            this_->onKey(key, scancode, action, mods);
        }
    }

    void glfwCursorPositionCallback(GLFWwindow* window, const double xpos, const double ypos)
    {
        auto* const this_ = static_cast<Window*>(glfwGetWindowUserPointer(window));
        if (this_->onCursorPosition) {
            this_->onCursorPosition(xpos, ypos);
        }
    }

    void glfwMouseButtonCallback(GLFWwindow* window, const int button, const int action, const int mods)
    {
        auto* const this_ = static_cast<Window*>(glfwGetWindowUserPointer(window));
        if (this_->onMouseButton) {
            this_->onMouseButton(button, action, mods);
        }
    }

    void glfwScrollCallback(GLFWwindow* window, const double xoffset, const double yoffset)
    {
        auto* const this_ = static_cast<Window*>(glfwGetWindowUserPointer(window));
        if (this_->onScroll) {
            this_->onScroll(xoffset, yoffset);
        }
    }

    std::vector<vk::QueueFamilyProperties>::const_iterator findQueue(
        const std::vector<vk::QueueFamilyProperties>& queueFamilies,
        const std::string& name,
        const vk::QueueFlags requiredBits,
        const vk::QueueFlags excludedBits)
    {
        const auto family = std::find_if(queueFamilies.begin(), queueFamilies.end(), [requiredBits, excludedBits](const vk::QueueFamilyProperties& queueFamily) {
            return queueFamily.queueCount > 0 && queueFamily.queueFlags & requiredBits && !(queueFamily.queueFlags & excludedBits);
        });

        if (family == queueFamilies.end()) {
            throw std::runtime_error("found no matching " + name + " queue");
        }

        return family;
    }

    std::vector<char> readFile(const std::string& filename)
    {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            throw std::runtime_error("failed to open file!");
        }

        size_t fileSize = (size_t)file.tellg();
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();

        return buffer;
    }

    /// <summary>
    /// For when you have to use vk::Image instead of vkr::Image.
    /// </summary>
    void transitionImageLayout(vk::CommandBuffer cmdBuf, vk::Image image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout)
    {
        vk::PipelineStageFlags srcStageMask = vk::PipelineStageFlagBits::eAllCommands;
        vk::PipelineStageFlags dstStageMask = vk::PipelineStageFlagBits::eAllCommands;

        vk::ImageMemoryBarrier imageMemoryBarrier{};
        imageMemoryBarrier
            .setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
            .setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
            .setImage(image)
            .setOldLayout(oldLayout)
            .setNewLayout(newLayout)
            .setSubresourceRange({ vk::ImageAspectFlagBits::eColor , 0, 1, 0, 1 });

        // Source layouts (old)
        switch (oldLayout) {
            case vk::ImageLayout::eUndefined:
                imageMemoryBarrier.srcAccessMask = {};
                break;
            case vk::ImageLayout::ePreinitialized:
                imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eHostWrite;
                break;
            case vk::ImageLayout::eColorAttachmentOptimal:
                imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
                break;
            case vk::ImageLayout::eDepthStencilAttachmentOptimal:
                imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentWrite;
                break;
            case vk::ImageLayout::eTransferSrcOptimal:
                imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
                break;
            case vk::ImageLayout::eTransferDstOptimal:
                imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
                break;
            case vk::ImageLayout::eShaderReadOnlyOptimal:
                imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eShaderRead;
                break;
            default:
                break;
        }

        // Target layouts (new)
        switch (newLayout) {
            case vk::ImageLayout::eTransferDstOptimal:
                imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
                break;
            case vk::ImageLayout::eTransferSrcOptimal:
                imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;
                break;
            case vk::ImageLayout::eColorAttachmentOptimal:
                imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
                break;
            case vk::ImageLayout::eDepthStencilAttachmentOptimal:
                imageMemoryBarrier.dstAccessMask = imageMemoryBarrier.dstAccessMask | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
                break;
            case vk::ImageLayout::eShaderReadOnlyOptimal:
                if (imageMemoryBarrier.srcAccessMask == vk::AccessFlags{}) {
                    imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eHostWrite | vk::AccessFlagBits::eTransferWrite;
                }
                imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
                break;
            default:
                break;
        }

        cmdBuf.pipelineBarrier(
            srcStageMask,      // srcStageMask
            dstStageMask,      // dstStageMask
            {},                // dependencyFlags
            {},                // memoryBarriers
            {},                // bufferMemoryBarriers
            imageMemoryBarrier // imageMemoryBarriers
        );
    }

    vk::TransformMatrixKHR toVkMatrix(const glm::mat4 transformMatrix)
    {
        const glm::mat4 transposedMatrix = glm::transpose(transformMatrix);
        std::array<std::array<float, 4>, 3> data;
        std::memcpy(&data, &transposedMatrix, sizeof(vk::TransformMatrixKHR));
        return vk::TransformMatrixKHR(data);
    }

    template <class T>
    uint32_t toU32(T value)
    {
        static_assert(std::is_arithmetic<T>::value, "T must be numeric");

        if (static_cast<uintmax_t>(value) > static_cast<uintmax_t>(std::numeric_limits<uint32_t>::max())) {
            throw std::runtime_error("toU32() failed, value is too big to be converted to uint32_t");
        }

        return static_cast<uint32_t>(value);
    }


    // Window
    Window::Window(const std::string& title, const uint32_t width, const uint32_t height,
                   bool cursorDisabled, bool fullscreen, bool resizable)
        : title(title), width(width), height(height)
        , cursorDisabled(cursorDisabled), fullscreen(fullscreen), resizable(resizable)
    {
        glfwSetErrorCallback(glfwErrorCallback);

        if (!glfwInit()) {
            throw std::runtime_error("glfwInit() failed");
        }

        if (!glfwVulkanSupported()) {
            throw std::runtime_error("glfwVulkanSupported() failed");
        }

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, resizable ? GLFW_TRUE : GLFW_FALSE);

        auto* const monitor = fullscreen ? glfwGetPrimaryMonitor() : nullptr;

        window = glfwCreateWindow(width, height, title.c_str(), monitor, nullptr);
        if (window == nullptr) {
            throw std::runtime_error("failed to create window");
        }

        if (cursorDisabled) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }

        glfwSetWindowUserPointer(window, this);
        glfwSetKeyCallback(window, glfwKeyCallback);
        glfwSetCursorPosCallback(window, glfwCursorPositionCallback);
        glfwSetMouseButtonCallback(window, glfwMouseButtonCallback);
        glfwSetScrollCallback(window, glfwScrollCallback);
    }

    // Instance
    Instance::Instance(const Window& window, const bool enableValidationLayers)
        : window(window)
        , enableValidationLayers(enableValidationLayers)
    {
        static vk::DynamicLoader dl;
        auto vkGetInstanceProcAddr = dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
        VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);

        const uint32_t version = VK_API_VERSION_1_2;
        checkVulkanMinimumVersion(version);

        auto extensions = window.getRequiredInstanceExtensions();

        if (enableValidationLayers) {
            validationLayers.push_back("VK_LAYER_KHRONOS_validation");
            checkVulkanValidationLayerSupport(validationLayers);

            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        vk::ApplicationInfo appInfo{ window.getTitle().c_str() , 0, "No Engine", 0, version };

        instance = vk::createInstanceUnique({ {}, &appInfo, validationLayers, extensions });
        VULKAN_HPP_DEFAULT_DISPATCHER.init(*instance);

        physicalDevices = instance->enumeratePhysicalDevices();

        extensionProperties = vk::enumerateInstanceExtensionProperties();

        if (enableValidationLayers) {
            createDebugMessenger();
        }
    }

    vk::UniqueSurfaceKHR Instance::createSurface() const
    {
        vk::SurfaceKHR surface = window.createWindowSurface(*instance);

        vk::ObjectDestroy<vk::Instance, VULKAN_HPP_DEFAULT_DISPATCHER_TYPE> _deleter(*instance);

        return vk::UniqueSurfaceKHR{ surface, _deleter };
    }

    vk::PhysicalDevice Instance::pickSuitablePhysicalDevice() const
    {
        const auto result = std::find_if(physicalDevices.begin(), physicalDevices.end(), [](const vk::PhysicalDevice& device) {
            const auto queueFamilies = device.getQueueFamilyProperties();

            const auto hasGraphicsQueue = std::find_if(queueFamilies.begin(), queueFamilies.end(), [](const vk::QueueFamilyProperties& queueFamily) {
                return queueFamily.queueCount > 0 && queueFamily.queueFlags & vk::QueueFlagBits::eGraphics;
            });

            return hasGraphicsQueue != queueFamilies.end();
        });

        if (result == physicalDevices.end()) {
            throw std::runtime_error("cannot find a suitable device");
        }

        return *result;
    }

    void Instance::createDebugMessenger()
    {
        vk::DebugUtilsMessageSeverityFlagsEXT severityFlags{ vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning
                                                           | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError };

        vk::DebugUtilsMessageTypeFlagsEXT messageTypeFlags{ vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral
                                                          | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance
                                                          | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation };

        messenger = instance->createDebugUtilsMessengerEXTUnique({ {}, severityFlags, messageTypeFlags, &debugUtilsMessengerCallback });
    }

    void Instance::checkVulkanMinimumVersion(uint32_t minVersion)
    {
        uint32_t version = vk::enumerateInstanceVersion();

        if (minVersion > version) {
            throw std::runtime_error("minimum required version not found");
        }
    }

    void Instance::checkVulkanValidationLayerSupport(const std::vector<const char*>& validationLayers)
    {
        const auto availableLayers = vk::enumerateInstanceLayerProperties();

        for (const char* layer : validationLayers) {
            auto result = std::find_if(availableLayers.begin(), availableLayers.end(), [layer](const vk::LayerProperties& layerProperties) {
                return strcmp(layer, layerProperties.layerName) == 0;
            });

            if (result == availableLayers.end()) {
                throw std::runtime_error("could not find the requested validation layer: '" + std::string(layer) + "'");
            }
        }
    }

    // Device
    Device::Device(const Instance& instance)
    {
        surface = instance.createSurface();
        physicalDevice = instance.pickSuitablePhysicalDevice();

        checkRequiredExtensions(physicalDevice);

        const auto queueFamilies = physicalDevice.getQueueFamilyProperties();

        // Find queues
        const auto graphicsFamily = findQueue(queueFamilies, "graphics", vk::QueueFlagBits::eGraphics, {});
        const auto computeFamily = findQueue(queueFamilies, "compute", vk::QueueFlagBits::eCompute, vk::QueueFlagBits::eGraphics);
        const auto transferFamily = findQueue(queueFamilies, "transfer", vk::QueueFlagBits::eTransfer, vk::QueueFlagBits::eGraphics | vk::QueueFlagBits::eCompute);

        // Find the presentation queue (usually the same as graphics queue).
        const auto presentFamily = std::find_if(queueFamilies.begin(), queueFamilies.end(), [&](const vk::QueueFamilyProperties& queueFamily) {
            VkBool32 presentSupport = false;
            const uint32_t i = static_cast<uint32_t>(&*queueFamilies.cbegin() - &queueFamily);
            presentSupport = physicalDevice.getSurfaceSupportKHR(i, *surface);
            return queueFamily.queueCount > 0 && presentSupport;
        });

        if (presentFamily == queueFamilies.end()) {
            throw std::runtime_error("found no presentation queue");
        }

        graphicsFamilyIndex = static_cast<uint32_t>(graphicsFamily - queueFamilies.begin());
        computeFamilyIndex = static_cast<uint32_t>(computeFamily - queueFamilies.begin());
        presentFamilyIndex = static_cast<uint32_t>(presentFamily - queueFamilies.begin());
        transferFamilyIndex = static_cast<uint32_t>(transferFamily - queueFamilies.begin());

        // Queues can be the same
        const std::set<uint32_t> uniqueQueueFamilies{
            graphicsFamilyIndex,
            computeFamilyIndex,
            presentFamilyIndex,
            transferFamilyIndex
        };

        // Create queues
        float queuePriority = 1.0f;
        std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
        for (uint32_t queueFamilyIndex : uniqueQueueFamilies) {
            queueCreateInfos.push_back({ {}, queueFamilyIndex, 1, &queuePriority });
        }

        vk::PhysicalDeviceFeatures deviceFeatures{};
        deviceFeatures.fillModeNonSolid = true;
        deviceFeatures.samplerAnisotropy = true;

        vk::PhysicalDeviceDescriptorIndexingFeaturesEXT indexingFeatures{};
        indexingFeatures.runtimeDescriptorArray = true;

        vk::DeviceCreateInfo createInfo{ {}, queueCreateInfos, instance.getValidationLayers(), requiredExtensions, &deviceFeatures };

        vk::StructureChain<vk::DeviceCreateInfo, vk::PhysicalDeviceDescriptorIndexingFeaturesEXT,
            vk::PhysicalDeviceBufferDeviceAddressFeatures, vk::PhysicalDeviceRayTracingPipelineFeaturesKHR,
            vk::PhysicalDeviceAccelerationStructureFeaturesKHR> createInfoChain{ createInfo, indexingFeatures, {true}, {true}, {true} };

        device = physicalDevice.createDeviceUnique(createInfoChain.get<vk::DeviceCreateInfo>());

        VULKAN_HPP_DEFAULT_DISPATCHER.init(*device);

        graphicsQueue = device->getQueue(graphicsFamilyIndex, 0);
        computeQueue = device->getQueue(computeFamilyIndex, 0);
        presentQueue = device->getQueue(presentFamilyIndex, 0);
        transferQueue = device->getQueue(transferFamilyIndex, 0);

        commandPool = device->createCommandPoolUnique({ vk::CommandPoolCreateFlagBits::eResetCommandBuffer, graphicsFamilyIndex });
    }

    uint32_t Device::findMemoryType(const uint32_t typeFilter, const vk::MemoryPropertyFlags properties) const
    {
        vk::PhysicalDeviceMemoryProperties memProperties = physicalDevice.getMemoryProperties();

        for (uint32_t i = 0; i != memProperties.memoryTypeCount; ++i) {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }

        throw std::runtime_error("failed to find suitable memory type");
    }

    vk::UniqueCommandBuffer Device::createCommandBuffer(vk::CommandBufferLevel level, bool begin, vk::CommandBufferUsageFlags usage) const
    {
        assert(commandPool);

        vk::UniqueCommandBuffer commandBuffer = std::move(device->allocateCommandBuffersUnique({ *commandPool , level, 1 }).front());

        if (begin) {
            commandBuffer->begin({ usage });
        }

        return commandBuffer;
    }

    void Device::submitCommandBuffer(vk::CommandBuffer& commandBuffer) const
    {
        commandBuffer.end();

        vk::UniqueFence fence = device->createFenceUnique({});

        vk::SubmitInfo submitInfo{};
        submitInfo.setCommandBuffers(commandBuffer);
        graphicsQueue.submit(submitInfo, fence.get());

        auto res = device->waitForFences(fence.get(), true, std::numeric_limits<uint64_t>::max());
        assert(res == vk::Result::eSuccess);
    }


    vk::UniquePipeline Device::createRayTracingPipeline(const DescriptorSets& descSets, const ShaderManager& shaderManager, uint32_t maxRecursionDepth)
    {
        auto result = device->createRayTracingPipelineKHRUnique(
            nullptr, nullptr,
            vk::RayTracingPipelineCreateInfoKHR{}
            .setStages(shaderManager.getStages())
            .setGroups(shaderManager.getRayTracingGroups())
            .setMaxPipelineRayRecursionDepth(maxRecursionDepth)
            .setLayout(descSets.getPipelineLayout())
        );
        if (result.result == vk::Result::eSuccess) {
            return std::move(result.value);
        }

        throw std::runtime_error("failed to create ray tracing pipeline.");
    }

    void Device::checkRequiredExtensions(vk::PhysicalDevice physicalDevice) const
    {
        const auto availableExtensions = physicalDevice.enumerateDeviceExtensionProperties();

        std::set<std::string> requiredExtensions(requiredExtensions.begin(), requiredExtensions.end());

        for (const auto& extension : availableExtensions) {
            requiredExtensions.erase(extension.extensionName);
        }

        if (!requiredExtensions.empty()) {
            bool first = true;
            std::string extensions;

            for (const auto& extension : requiredExtensions) {
                if (!first) {
                    extensions += ", ";
                }

                extensions += extension;
                first = false;
            }

            throw std::runtime_error("missing required extensions: " + extensions);
        }
    }

    // SwapChain
    SwapChain::SwapChain(const Device& device, const Window& window)
        : device(device)
        , physicalDevice(device.getPhysicalDevice())
    {
        const auto details = querySwapChainSupport(device.getPhysicalDevice(), device.getSurface());
        if (details.formats.empty() || details.presentModes.empty()) {
            throw std::runtime_error("empty swap chain support");
        }

        const auto& surface = device.getSurface();

        const auto surfaceFormat = chooseSwapSurfaceFormat(details.formats);
        presentMode = chooseSwapPresentMode(details.presentModes);
        extent = chooseSwapExtent(window, details.capabilities);
        imageCount = chooseImageCount(details.capabilities);

        // Create swap chain
        vk::SwapchainCreateInfoKHR createInfo{};
        createInfo.surface = surface;
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst;
        createInfo.imageSharingMode = vk::SharingMode::eExclusive;
        createInfo.preTransform = details.capabilities.currentTransform;
        createInfo.presentMode = presentMode;
        createInfo.clipped = true;

        if (device.getGraphicsFamilyIndex() != device.getPresentFamilyIndex()) {
            uint32_t queueFamilyIndices[] = { device.getGraphicsFamilyIndex(), device.getPresentFamilyIndex() };
            createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        }

        swapChain = device.getHandle().createSwapchainKHRUnique(createInfo);

        format = surfaceFormat.format;
        images = device.getHandle().getSwapchainImagesKHR(*swapChain);

        // Create image views
        imageViews.reserve(images.size());
        for (const auto image : images) {
            vk::ImageViewCreateInfo viewInfo{ {}, image, vk::ImageViewType::e2D, format };
            viewInfo.subresourceRange = { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 };
            imageViews.push_back(device.getHandle().createImageViewUnique(viewInfo));
        }

        // Create semaphores
        imageAvailableSemaphores.resize(maxFramesInFlight);
        renderFinishedSemaphores.resize(maxFramesInFlight);
        inFlightFences.resize(maxFramesInFlight);
        imagesInFlight.resize(images.size());

        for (size_t i = 0; i < maxFramesInFlight; i++) {
            imageAvailableSemaphores[i] = device.getHandle().createSemaphoreUnique({});
            renderFinishedSemaphores[i] = device.getHandle().createSemaphoreUnique({});
            inFlightFences[i] = device.getHandle().createFence({ vk::FenceCreateFlagBits::eSignaled });
        }
    }

    SwapChain::SupportDetails SwapChain::querySwapChainSupport(vk::PhysicalDevice physicalDevice, const vk::SurfaceKHR surface)
    {
        SupportDetails details;
        details.capabilities = physicalDevice.getSurfaceCapabilitiesKHR(surface);
        details.formats = physicalDevice.getSurfaceFormatsKHR(surface);
        details.presentModes = physicalDevice.getSurfacePresentModesKHR(surface);
        return details;
    }

    vk::SurfaceFormatKHR SwapChain::chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& formats)
    {
        if (formats.size() == 1 && formats[0].format == vk::Format::eUndefined) {
            return { vk::Format::eB8G8R8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear };
        }

        for (const auto& format : formats) {
            if (format.format == vk::Format::eB8G8R8A8Unorm && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
                return format;
            }
        }

        throw std::runtime_error("found no suitable surface format");
    }

    vk::PresentModeKHR SwapChain::chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& presentModes)
    {
        for (const auto& availablePresentMode : presentModes) {
            if (availablePresentMode == vk::PresentModeKHR::eFifoRelaxed) {
                return availablePresentMode;
            }
        }

        return vk::PresentModeKHR::eFifo;
    }

    vk::Extent2D SwapChain::chooseSwapExtent(const Window& window, const vk::SurfaceCapabilitiesKHR& capabilities)
    {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return capabilities.currentExtent;
        }

        auto actualExtent = window.getFramebufferSize();

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }

    uint32_t SwapChain::chooseImageCount(const vk::SurfaceCapabilitiesKHR& capabilities)
    {
        uint32_t imageCount = capabilities.minImageCount + 1;

        if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
            imageCount = capabilities.maxImageCount;
        }

        return imageCount;
    }

    std::unique_ptr<Image> SwapChain::createStorageImage() const
    {
        auto image = std::make_unique<Image>(device, extent, format, vk::MemoryPropertyFlagBits::eDeviceLocal, vk::ImageAspectFlagBits::eColor,
                                             vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eTransferSrc);

        auto commandBuffer = device.createCommandBuffer();
        transitionImageLayout(commandBuffer.get(), image->getHandle(), vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral);
        device.submitCommandBuffer(commandBuffer.get());

        image->setLayout(vk::ImageLayout::eGeneral);

        return image;
    }

    void SwapChain::initDrawCommandBuffers(vk::Pipeline pipeline, const DescriptorSets& descSets, const ShaderManager& shaderManager, vkr::Image& storageImage)
    {
        assert(images.size());

        drawCmdBufs = device.getHandle().allocateCommandBuffersUnique(
            vk::CommandBufferAllocateInfo{}
            .setCommandPool(device.getCommandPool())
            .setLevel(vk::CommandBufferLevel::ePrimary)
            .setCommandBufferCount(static_cast<uint32_t>(images.size()))
        );

        for (uint32_t i = 0; i < drawCmdBufs.size(); ++i) {
            drawCmdBufs[i]->begin({ vk::CommandBufferUsageFlagBits::eSimultaneousUse });

            drawCmdBufs[i]->bindPipeline(vk::PipelineBindPoint::eRayTracingKHR, pipeline);

            drawCmdBufs[i]->bindDescriptorSets(
                vk::PipelineBindPoint::eRayTracingKHR, // pipelineBindPoint
                descSets.getPipelineLayout(),          // layout
                0,                                     // firstSet
                descSets.getDescriptorSets(),          // descriptorSets
                nullptr                                // dynamicOffsets
            );

            drawCmdBufs[i]->traceRaysKHR(
                shaderManager.getRaygenRegion(), // raygenShaderBindingTable
                shaderManager.getMissRegion(),   // missShaderBindingTable
                shaderManager.getHitRegion(),    // hitShaderBindingTable
                {},                              // callableShaderBindingTable
                extent.width,  // width
                extent.height, // height
                1              // depth
            );

            transitionImageLayout(*drawCmdBufs[i], images.at(i), vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
            transitionImageLayout(*drawCmdBufs[i], storageImage.getHandle(), vk::ImageLayout::eGeneral, vk::ImageLayout::eTransferSrcOptimal);

            vk::ImageCopy copyRegion{};
            copyRegion.setSrcSubresource({ vk::ImageAspectFlagBits::eColor, 0, 0, 1 });
            copyRegion.setSrcOffset({ 0, 0, 0 });
            copyRegion.setDstSubresource({ vk::ImageAspectFlagBits::eColor, 0, 0, 1 });
            copyRegion.setDstOffset({ 0, 0, 0 });
            copyRegion.setExtent({ extent.width, extent.height, 1 });

            drawCmdBufs[i]->copyImage(
                storageImage.getHandle(),             // srcImage
                vk::ImageLayout::eTransferSrcOptimal, // srcImageLayout
                images[i],                            // dstImage
                vk::ImageLayout::eTransferDstOptimal, // dstImageLayout
                copyRegion                            // regions
            );

            transitionImageLayout(*drawCmdBufs[i], images.at(i), vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::ePresentSrcKHR);
            transitionImageLayout(*drawCmdBufs[i], storageImage.getHandle(), vk::ImageLayout::eTransferSrcOptimal, vk::ImageLayout::eGeneral);

            drawCmdBufs[i]->end();
        }
    }

    uint32_t SwapChain::acquireNextImage()
    {
        auto result = device.getHandle().acquireNextImageKHR(
            swapChain.get(),                             // swapchain
            std::numeric_limits<uint64_t>::max(),        // timeout
            imageAvailableSemaphores[currentFrame].get() // semaphore
        );

        if (result.result == vk::Result::eSuccess) {
            return result.value;
        }

        throw std::runtime_error("failed to acquire next image");
    }

    void SwapChain::draw()
    {
        auto res = device.getHandle().waitForFences(inFlightFences[currentFrame], true, std::numeric_limits<uint64_t>::max());
        assert(res == vk::Result::eSuccess);

        uint32_t imageIndex = acquireNextImage();

        if (imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
            res = device.getHandle().waitForFences(imagesInFlight[imageIndex], true, std::numeric_limits<uint64_t>::max());
            assert(res == vk::Result::eSuccess);
        }
        imagesInFlight[imageIndex] = inFlightFences[currentFrame];

        device.getHandle().resetFences(inFlightFences[currentFrame]);

        vk::PipelineStageFlags waitStage{ vk::PipelineStageFlagBits::eRayTracingShaderKHR };
        device.getGraphicsQueue().submit(
            vk::SubmitInfo{}
            .setWaitSemaphores(imageAvailableSemaphores[currentFrame].get())
            .setWaitDstStageMask(waitStage)
            .setCommandBuffers(drawCmdBufs[imageIndex].get())
            .setSignalSemaphores(renderFinishedSemaphores[currentFrame].get()),
            inFlightFences[currentFrame]
        );

        res = device.getGraphicsQueue().presentKHR(
            vk::PresentInfoKHR{}
            .setWaitSemaphores(renderFinishedSemaphores[currentFrame].get())
            .setSwapchains(swapChain.get())
            .setImageIndices(imageIndex)
        );
        assert(res == vk::Result::eSuccess);

        currentFrame = (currentFrame + 1) % maxFramesInFlight;
    }

    // Image
    Image::Image(const Device& device, vk::Extent2D extent, vk::Format format,
                 vk::ImageTiling tiling, vk::ImageUsageFlags usage)
        : extent(extent), format(format), imageLayout(vk::ImageLayout::eUndefined)
    {
        vk::ImageCreateInfo imageInfo = {};
        imageInfo.imageType = vk::ImageType::e2D;
        imageInfo.extent.width = extent.width;
        imageInfo.extent.height = extent.height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = format;
        imageInfo.tiling = tiling;
        imageInfo.usage = usage;

        image = device.getHandle().createImageUnique(imageInfo);
    }

    Image::Image(const Device& device, vk::Extent2D extent, vk::Format format,
                 vk::MemoryPropertyFlags properties, vk::ImageAspectFlags aspectFlags,
                 vk::ImageTiling tiling, vk::ImageUsageFlags usage)
        : extent(extent), format(format), imageLayout(vk::ImageLayout::eUndefined)
    {
        vk::ImageCreateInfo imageInfo = {};
        imageInfo.imageType = vk::ImageType::e2D;
        imageInfo.extent.width = extent.width;
        imageInfo.extent.height = extent.height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = format;
        imageInfo.tiling = tiling;
        imageInfo.usage = usage;

        image = device.getHandle().createImageUnique(imageInfo);

        const auto requirements = device.getHandle().getImageMemoryRequirements(*image);
        auto memoryTypeIndex = device.findMemoryType(requirements.memoryTypeBits, properties);
        memory = device.getHandle().allocateMemoryUnique({ requirements.size, memoryTypeIndex });

        device.getHandle().bindImageMemory(*image, *memory, 0);

        vk::ImageViewCreateInfo createInfo{ {}, *image, vk::ImageViewType::e2D, format };
        createInfo.subresourceRange = { aspectFlags , 0, 1, 0, 1 };
        view = device.getHandle().createImageViewUnique(createInfo);
    }


    vk::DescriptorImageInfo Image::createDescriptorInfo() const
    {
        return { {}, *view, imageLayout };
    }

    void Image::copyFrom(vk::CommandBuffer& cmdBuf, const Buffer& buffer)
    {
        vk::BufferImageCopy region{};
        region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageExtent = vk::Extent3D{ extent.width, extent.height, 1 };

        cmdBuf.copyBufferToImage(buffer.getHandle(), *image, vk::ImageLayout::eTransferDstOptimal, region);
    }

    void Image::copyFrom(const Device& device, const Buffer& buffer)
    {
        auto cmdBuf = device.createCommandBuffer();

        copyFrom(*cmdBuf, buffer);

        device.submitCommandBuffer(*cmdBuf);
    }

    void Image::transitionImageLayout(vk::CommandBuffer& cmdBuf, vk::ImageLayout newLayout)
    {
        vk::PipelineStageFlags srcStageMask = vk::PipelineStageFlagBits::eAllCommands;
        vk::PipelineStageFlags dstStageMask = vk::PipelineStageFlagBits::eAllCommands;

        vk::ImageMemoryBarrier imageMemoryBarrier{};
        imageMemoryBarrier
            .setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
            .setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
            .setImage(*image)
            .setOldLayout(imageLayout)
            .setNewLayout(newLayout)
            .setSubresourceRange({ vk::ImageAspectFlagBits::eColor , 0, 1, 0, 1 });

        // Source layouts (old)
        switch (imageLayout) {
            case vk::ImageLayout::eUndefined:
                imageMemoryBarrier.srcAccessMask = {};
                break;
            case vk::ImageLayout::ePreinitialized:
                imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eHostWrite;
                break;
            case vk::ImageLayout::eColorAttachmentOptimal:
                imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
                break;
            case vk::ImageLayout::eDepthStencilAttachmentOptimal:
                imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentWrite;
                break;
            case vk::ImageLayout::eTransferSrcOptimal:
                imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
                break;
            case vk::ImageLayout::eTransferDstOptimal:
                imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
                break;
            case vk::ImageLayout::eShaderReadOnlyOptimal:
                imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eShaderRead;
                break;
            default:
                break;
        }

        // Target layouts (new)
        switch (newLayout) {
            case vk::ImageLayout::eTransferDstOptimal:
                imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
                break;
            case vk::ImageLayout::eTransferSrcOptimal:
                imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;
                break;
            case vk::ImageLayout::eColorAttachmentOptimal:
                imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
                break;
            case vk::ImageLayout::eDepthStencilAttachmentOptimal:
                imageMemoryBarrier.dstAccessMask = imageMemoryBarrier.dstAccessMask | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
                break;
            case vk::ImageLayout::eShaderReadOnlyOptimal:
                if (imageMemoryBarrier.srcAccessMask == vk::AccessFlags{}) {
                    imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eHostWrite | vk::AccessFlagBits::eTransferWrite;
                }
                imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
                break;
            default:
                break;
        }

        cmdBuf.pipelineBarrier(
            srcStageMask,      // srcStageMask
            dstStageMask,      // dstStageMask
            {},                // dependencyFlags
            {},                // memoryBarriers
            {},                // bufferMemoryBarriers
            imageMemoryBarrier // imageMemoryBarriers
        );

        imageLayout = newLayout;
    }


    // Buffer
    Buffer::Buffer(const Device& device, vk::DeviceSize size, vk::BufferUsageFlags usage)
        : device(device.getHandle()), size(size)
    {
        buffer = device.getHandle().createBufferUnique({ {}, size, usage });
    }

    Buffer::Buffer(const Device& device, vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties)
        : device(device.getHandle()), size(size)
    {
        // Create buffer
        buffer = device.getHandle().createBufferUnique({ {}, size, usage });

        // Find memory requirements
        const auto requirements = device.getHandle().getBufferMemoryRequirements(*buffer);

        vk::MemoryAllocateFlagsInfo flagsInfo{};
        if (usage & vk::BufferUsageFlagBits::eShaderDeviceAddress) {
            flagsInfo.flags = vk::MemoryAllocateFlagBits::eDeviceAddress;
        }

        // Allocate memory
        vk::MemoryAllocateInfo allocInfo{};
        allocInfo.allocationSize = requirements.size;
        allocInfo.memoryTypeIndex = device.findMemoryType(requirements.memoryTypeBits, properties);
        allocInfo.pNext = &flagsInfo;
        memory = device.getHandle().allocateMemoryUnique(allocInfo);

        // Bind memory to buffer
        device.getHandle().bindBufferMemory(*buffer, *memory, 0);

        if (usage & vk::BufferUsageFlagBits::eShaderDeviceAddress) {
            vk::BufferDeviceAddressInfoKHR bufferDeviceAI{ *buffer };
            deviceAddress = device.getHandle().getBufferAddressKHR(&bufferDeviceAI);
        }
    }

    Buffer::Buffer(const Device& device, vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, void* data)
        : device(device.getHandle()), size(size)
    {
        // Create buffer
        if (properties & vk::MemoryPropertyFlagBits::eDeviceLocal) {
            usage = usage | vk::BufferUsageFlagBits::eTransferDst;
        }
        buffer = device.getHandle().createBufferUnique({ {}, size, usage });

        // Find memory requirements
        const auto requirements = device.getHandle().getBufferMemoryRequirements(*buffer);

        vk::MemoryAllocateFlagsInfo flagsInfo{};
        if (usage & vk::BufferUsageFlagBits::eShaderDeviceAddress) {
            flagsInfo.flags = vk::MemoryAllocateFlagBits::eDeviceAddress;
        }

        // Allocate memory
        vk::MemoryAllocateInfo allocInfo{};
        allocInfo.allocationSize = requirements.size;
        allocInfo.memoryTypeIndex = device.findMemoryType(requirements.memoryTypeBits, properties);
        allocInfo.pNext = &flagsInfo;
        memory = device.getHandle().allocateMemoryUnique(allocInfo);

        // Bind memory to buffer
        device.getHandle().bindBufferMemory(*buffer, *memory, 0);

        if (usage & vk::BufferUsageFlagBits::eShaderDeviceAddress) {
            vk::BufferDeviceAddressInfoKHR bufferDeviceAI{ *buffer };
            deviceAddress = device.getHandle().getBufferAddressKHR(&bufferDeviceAI);
        }

        if (properties & vk::MemoryPropertyFlagBits::eHostVisible) {
            // If it is a host buffer, just copy the data.
            //void* dataPtr = device.getHandle().mapMemory(*memory, 0, size);
            //memcpy(dataPtr, data, static_cast<size_t>(size));

            map();
            memcpy(mapped, data, static_cast<size_t>(size));

            if (!(properties & vk::MemoryPropertyFlagBits::eHostCoherent)) {
                vk::MappedMemoryRange mapped_range{};
                mapped_range.memory = *memory;
                mapped_range.offset = 0;
                mapped_range.size = size;
                device.getHandle().flushMappedMemoryRanges(mapped_range);
            }

            //device.getHandle().unmapMemory(*memory);

        } else if (properties & vk::MemoryPropertyFlagBits::eDeviceLocal) {
            // If it is a device buffer, send it to the device with a copy command via the staging buffer.
            auto stagingBuffer = Buffer(device, size, usage | vk::BufferUsageFlagBits::eTransferSrc,
                                        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, data);

            vk::BufferCopy region{ 0, 0, size };
            auto commandBuffer = device.createCommandBuffer();
            commandBuffer->copyBuffer(stagingBuffer.getHandle(), *buffer, region);
            device.submitCommandBuffer(*commandBuffer);
        }
    }


    void Buffer::copyFrom(const Device& device, const Buffer& src)
    {
        auto commandBuffer = device.createCommandBuffer();

        commandBuffer->copyBuffer(src.getHandle(), *buffer, { 0, 0, src.getSize() });

        device.submitCommandBuffer(*commandBuffer);
    }

    vk::DescriptorBufferInfo Buffer::createDescriptorInfo() const
    {
        return vk::DescriptorBufferInfo{ *buffer, 0, size };
    }

    void Buffer::map()
    {
        mapped = device.mapMemory(*memory, 0, size);
    }

    void Buffer::map(vk::DeviceSize size, vk::DeviceSize offset)
    {
        mapped = device.mapMemory(*memory, offset, size);
    }

    void Buffer::unmap()
    {
        if (mapped) {
            device.unmapMemory(*memory);
            mapped = nullptr;
        }
    }

    void Buffer::copy(void* data)
    {
        memcpy(mapped, data, size);
    }


    // DescriptorSetBindings
    void DescriptorSetBindings::addBindging(uint32_t binding, vk::DescriptorType type, uint32_t count,
                                            vk::ShaderStageFlags stageFlags, const vk::Sampler* pImmutableSampler/*= nullptr*/)
    {
        bindings.push_back({ binding, type, count, stageFlags, pImmutableSampler });
    }

    vk::UniqueDescriptorSetLayout DescriptorSetBindings::createLayout(const Device& device, vk::DescriptorSetLayoutCreateFlags flags) const
    {
        return device.getHandle().createDescriptorSetLayoutUnique({ flags, bindings });
    }

    void DescriptorSetBindings::addRequiredPoolSizes(std::vector<vk::DescriptorPoolSize>& poolSizes) const
    {
        for (auto it = bindings.cbegin(); it != bindings.cend(); ++it) {
            bool found = false;
            for (auto itpool = poolSizes.begin(); itpool != poolSizes.end(); ++itpool) {
                if (itpool->type == it->descriptorType) {
                    itpool->descriptorCount += it->descriptorCount;
                    found = true;
                    break;
                }
            }
            if (!found) {
                poolSizes.push_back({ it->descriptorType, it->descriptorCount });
            }
        }
    }

    vk::WriteDescriptorSet DescriptorSetBindings::makeWrite(vk::DescriptorSet dstSet, uint32_t dstBinding,
                                                            const vk::DescriptorImageInfo* pImageInfo, uint32_t arrayElement) const
    {
        for (const auto& binding : bindings) {
            if (binding.binding == dstBinding) {
                vk::WriteDescriptorSet write{ dstSet, dstBinding, arrayElement,
                    binding.descriptorCount, binding.descriptorType };
                write.pImageInfo = pImageInfo;

                return write;
            }
        }
    }

    vk::WriteDescriptorSet DescriptorSetBindings::makeWrite(vk::DescriptorSet dstSet, uint32_t dstBinding,
                                                            const vk::DescriptorBufferInfo* pBufferInfo, uint32_t arrayElement) const
    {
        for (const auto& binding : bindings) {
            if (binding.binding == dstBinding) {
                vk::WriteDescriptorSet write{ dstSet, dstBinding, arrayElement,
                    binding.descriptorCount, binding.descriptorType };
                write.pBufferInfo = pBufferInfo;

                return write;
            }
        }
    }

    vk::WriteDescriptorSet DescriptorSetBindings::makeWrite(vk::DescriptorSet dstSet, uint32_t dstBinding,
                                                            const vk::WriteDescriptorSetAccelerationStructureKHR* pASInfo, uint32_t arrayElement) const
    {
        for (const auto& binding : bindings) {
            if (binding.binding == dstBinding) {
                vk::WriteDescriptorSet write{ dstSet, dstBinding, arrayElement,
                    binding.descriptorCount, binding.descriptorType };
                write.pNext = pASInfo;

                return write;
            }
        }
    }

    // DescriptorSets
    DescriptorSets::DescriptorSets(const Device& device, uint32_t numSets /*= 1*/)
        : device(device)
        , numSets(numSets)
    {
        assert(numSets > 0);

        for (uint32_t i = 0; i < numSets; i++) {
            bindingsArray.push_back(std::make_unique<DescriptorSetBindings>());
        }
    }

    std::vector<vk::DescriptorSet> DescriptorSets::getDescriptorSets() const
    {
        std::vector<vk::DescriptorSet> rawDescSets;
        for (auto& descSet : descSets) {
            rawDescSets.push_back(*descSet);
        }
        return rawDescSets;
    }

    std::vector<vk::DescriptorSetLayout> DescriptorSets::getDescriptorSetLayouts() const
    {
        std::vector<vk::DescriptorSetLayout> rawDescSetLayouts;
        for (auto& descSetLayout : descSetLayouts) {
            rawDescSetLayouts.push_back(*descSetLayout);
        }
        return rawDescSetLayouts;
    }

    void DescriptorSets::addBindging(uint32_t setIndex, uint32_t binding, vk::DescriptorType type, uint32_t count,
                                     vk::ShaderStageFlags stageFlags, const vk::Sampler* pImmutableSampler /*= nullptr*/)
    {
        assert(setIndex < numSets);

        bindingsArray[setIndex]->addBindging(binding, type, count, stageFlags, pImmutableSampler);
    }

    void DescriptorSets::initPipelineLayout()
    {
        for (auto& bindings : bindingsArray) {
            descSetLayouts.push_back(bindings->createLayout(device));
        }

        pipeLayout = device.getHandle().createPipelineLayoutUnique({ {}, getDescriptorSetLayouts() });
    }

    vk::PipelineLayout DescriptorSets::createPipelineLayout()
    {
        initPipelineLayout();
        return *pipeLayout;
    }

    void DescriptorSets::allocate()
    {
        assert(!descSetLayouts.empty());

        // Create Desc Pool
        std::vector<vk::DescriptorPoolSize> poolSizes;
        for (const auto& bindings : bindingsArray) {
            bindings->addRequiredPoolSizes(poolSizes);
        }
        descPool = device.getHandle().createDescriptorPoolUnique({ vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet, numSets, poolSizes });

        // Allocate Desc Sets
        descSets = device.getHandle().allocateDescriptorSetsUnique({ *descPool, getDescriptorSetLayouts() });
    }

    // ShaderManager
    vk::UniqueShaderModule ShaderManager::createShaderModule(const std::string& filename)
    {
        const std::vector<char> code = readFile(filename);

        return device.getHandle().createShaderModuleUnique({ {}, code.size(), reinterpret_cast<const uint32_t*>(code.data()) });
    }

    void ShaderManager::addShader(const std::string& filename, vk::ShaderStageFlagBits stage, const char* pName,
                                  vk::RayTracingShaderGroupTypeKHR groupType)
    {
        modules.push_back(createShaderModule(filename));
        stages.push_back({ {}, stage, *modules.back(), pName });

        vk::RayTracingShaderGroupCreateInfoKHR groupInfo{ groupType,
            VK_SHADER_UNUSED_KHR, VK_SHADER_UNUSED_KHR, VK_SHADER_UNUSED_KHR, VK_SHADER_UNUSED_KHR };
        uint32_t index = static_cast<uint32_t>(stages.size() - 1);
        switch (groupType) {
            case vk::RayTracingShaderGroupTypeKHR::eGeneral:
                groupInfo.generalShader = index;
                break;
            case vk::RayTracingShaderGroupTypeKHR::eTrianglesHitGroup:
                groupInfo.closestHitShader = index;
                break;
            case vk::RayTracingShaderGroupTypeKHR::eProceduralHitGroup:
                break;
            default:
                break;
        }
        rtGroups.push_back(groupInfo);
    }

    void ShaderManager::initShaderBindingTable(const vk::Pipeline& pipeline, uint32_t raygenOffset, uint32_t missOffset, uint32_t hitOffset)
    {
        // Get Ray Tracing Properties
        auto properties = device.getPhysicalDevice().getProperties2<vk::PhysicalDeviceProperties2, vk::PhysicalDeviceRayTracingPipelinePropertiesKHR>();
        auto rtProperties = properties.get<vk::PhysicalDeviceRayTracingPipelinePropertiesKHR>();

        // Calc SBT size
        const uint32_t handleSize = rtProperties.shaderGroupHandleSize;
        const uint32_t handleSizeAligned = rtProperties.shaderGroupHandleAlignment;
        const uint32_t groupCount = static_cast<uint32_t>(rtGroups.size());
        const uint32_t sbtSize = groupCount * handleSizeAligned;

        using vkbu = vk::BufferUsageFlagBits;
        using vkmp = vk::MemoryPropertyFlagBits;
        const vk::BufferUsageFlags usage = vkbu::eShaderBindingTableKHR | vkbu::eShaderDeviceAddress;
        const vk::MemoryPropertyFlags memoryProperty = vkmp::eHostVisible | vkmp::eHostCoherent;

        // Get shader group handles
        std::vector<uint8_t> shaderHandleStorage(sbtSize);
        auto result = device.getHandle().getRayTracingShaderGroupHandlesKHR(pipeline, 0, groupCount, static_cast<size_t>(sbtSize),
                                                                            shaderHandleStorage.data());
        if (result != vk::Result::eSuccess) {
            throw std::runtime_error("failed to get ray tracing shader group handles.");
        }

        // Create SBT Buffers
        raygenShaderBindingTable = std::make_unique<Buffer>(device, handleSize, usage, memoryProperty,
                                                            shaderHandleStorage.data() + static_cast<uint64_t>(raygenOffset) * handleSizeAligned);
        missShaderBindingTable = std::make_unique<Buffer>(device, handleSize, usage, memoryProperty,
                                                          shaderHandleStorage.data() + static_cast<uint64_t>(missOffset) * handleSizeAligned);
        hitShaderBindingTable = std::make_unique<Buffer>(device, handleSize, usage, memoryProperty,
                                                         shaderHandleStorage.data() + static_cast<uint64_t>(hitOffset) * handleSizeAligned);

        raygenRegion.setDeviceAddress(raygenShaderBindingTable->getDeviceAddress());
        raygenRegion.setStride(handleSizeAligned);
        raygenRegion.setSize(handleSizeAligned);

        missRegion.setDeviceAddress(missShaderBindingTable->getDeviceAddress());
        missRegion.setStride(handleSizeAligned);
        missRegion.setSize(handleSizeAligned);

        hitRegion.setDeviceAddress(hitShaderBindingTable->getDeviceAddress());
        hitRegion.setStride(handleSizeAligned);
        hitRegion.setSize(handleSizeAligned);
    }

    // AccelerationStructure
    void AccelerationStructure::build(const Device& device, vk::AccelerationStructureGeometryKHR& geometry,
                                      const vk::AccelerationStructureTypeKHR& asType, uint32_t primitiveCount)
    {
        vk::AccelerationStructureBuildGeometryInfoKHR buildGeometryInfo{};
        buildGeometryInfo.setType(asType);
        buildGeometryInfo.setFlags(vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace);
        buildGeometryInfo.setGeometries(geometry);

        auto buildSizesInfo = device.getHandle().getAccelerationStructureBuildSizesKHR(
            vk::AccelerationStructureBuildTypeKHR::eDevice, buildGeometryInfo, primitiveCount);

        createBuffer(device, buildSizesInfo);

        accelerationStructure = device.getHandle().createAccelerationStructureKHRUnique(
            vk::AccelerationStructureCreateInfoKHR{}
            .setBuffer(buffer->getHandle())
            .setSize(buildSizesInfo.accelerationStructureSize)
            .setType(asType)
        );

        Buffer scratchBuffer{ device, buildSizesInfo.buildScratchSize,
            vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress,
            vk::MemoryPropertyFlagBits::eDeviceLocal };

        buildGeometryInfo.setDstAccelerationStructure(*accelerationStructure);
        buildGeometryInfo.setScratchData(scratchBuffer.getDeviceAddress());

        vk::AccelerationStructureBuildRangeInfoKHR accelerationStructureBuildRangeInfo{};
        accelerationStructureBuildRangeInfo
            .setPrimitiveCount(primitiveCount)
            .setPrimitiveOffset(0)
            .setFirstVertex(0)
            .setTransformOffset(0);

        auto commandBuffer = device.createCommandBuffer();
        commandBuffer->buildAccelerationStructuresKHR(buildGeometryInfo, &accelerationStructureBuildRangeInfo);
        device.submitCommandBuffer(*commandBuffer);

        deviceAddress = device.getHandle().getAccelerationStructureAddressKHR({ *accelerationStructure });
    }

    void AccelerationStructure::createBuffer(const Device& device, vk::AccelerationStructureBuildSizesInfoKHR buildSizesInfo)
    {
        auto size = buildSizesInfo.accelerationStructureSize;
        auto usage = vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR | vk::BufferUsageFlagBits::eShaderDeviceAddress;
        buffer = std::make_unique<Buffer>(device, size, usage, vk::MemoryPropertyFlagBits::eDeviceLocal);
    }


    BottomLevelAccelerationStructure::BottomLevelAccelerationStructure(const Device& device, const Mesh& mesh)
    {
        vk::BufferDeviceAddressInfoKHR vertexAddressInfo{ mesh.vertexBuffer->getHandle() };
        auto vertexAddress = device.getHandle().getBufferAddressKHR(&vertexAddressInfo);

        vk::BufferDeviceAddressInfoKHR indexAddressInfo{ mesh.indexBuffer->getHandle() };
        auto indexAddress = device.getHandle().getBufferAddressKHR(&indexAddressInfo);

        vk::AccelerationStructureGeometryTrianglesDataKHR triangleData{};
        triangleData.setVertexFormat(vk::Format::eR32G32B32Sfloat);
        triangleData.setVertexData(vertexAddress);
        triangleData.setVertexStride(sizeof(Vertex));
        triangleData.setMaxVertex(mesh.vertices.size());
        triangleData.setIndexType(vk::IndexType::eUint32);
        triangleData.setIndexData(indexAddress);

        vk::AccelerationStructureGeometryKHR geometry{};
        geometry.setGeometryType(vk::GeometryTypeKHR::eTriangles);
        geometry.setGeometry({ triangleData });
        geometry.setFlags(vk::GeometryFlagBitsKHR::eOpaque);

        uint32_t triangleCount = static_cast<uint32_t>(mesh.indices.size() / 3);
        build(device, geometry, vk::AccelerationStructureTypeKHR::eBottomLevel, triangleCount);
    }

    // TopLevelAccelerationStructure
    TopLevelAccelerationStructure::TopLevelAccelerationStructure(const Device& device, BottomLevelAccelerationStructure& blas, AccelerationStructureInstance& instance)
    {
        vk::AccelerationStructureInstanceKHR asInstance{};
        asInstance.setTransform(toVkMatrix(instance.transformMatrix));
        asInstance.setInstanceCustomIndex(instance.modelIndex);
        asInstance.setMask(0xFF);
        asInstance.setInstanceShaderBindingTableRecordOffset(0);
        asInstance.setFlags(vk::GeometryInstanceFlagBitsKHR::eTriangleFacingCullDisable);
        asInstance.setAccelerationStructureReference(blas.getDeviceAddress());

        vk::DeviceSize size{ sizeof(VkAccelerationStructureInstanceKHR) };
        Buffer instancesBuffer{ device, size,
            vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR | vk::BufferUsageFlagBits::eShaderDeviceAddress,
            vk::MemoryPropertyFlagBits::eDeviceLocal, &asInstance };

        vk::AccelerationStructureGeometryInstancesDataKHR instancesData{};
        instancesData.setArrayOfPointers(false);
        instancesData.setData(instancesBuffer.getDeviceAddress());

        vk::AccelerationStructureGeometryKHR geometry{};
        geometry.setGeometryType(vk::GeometryTypeKHR::eInstances);
        geometry.setGeometry({ instancesData });
        geometry.setFlags(vk::GeometryFlagBitsKHR::eOpaque);

        uint32_t instanceCount = 1;
        build(device, geometry, vk::AccelerationStructureTypeKHR::eTopLevel, instanceCount);
    }

} // vkr

namespace vkr
{
    void Mesh::create(const Device& device, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
    {
        this->vertices = vertices;
        this->indices = indices;

        using vkbu = vk::BufferUsageFlagBits;
        using vkmp = vk::MemoryPropertyFlagBits;

        vk::BufferUsageFlags usage{ vkbu::eAccelerationStructureBuildInputReadOnlyKHR
                                  | vkbu::eStorageBuffer
                                  | vkbu::eShaderDeviceAddress
                                  | vkbu::eTransferDst };

        vk::MemoryPropertyFlags properties{ vkmp::eDeviceLocal };

        uint64_t vertexBufferSize = vertices.size() * sizeof(Vertex);
        vertexBuffer = std::make_unique<Buffer>(device, vertexBufferSize, usage, properties, (void*)vertices.data());

        uint64_t indexBufferSize = indices.size() * sizeof(uint32_t);
        indexBuffer = std::make_unique<Buffer>(device, indexBufferSize, usage, properties, (void*)indices.data());
    }

    void Model::loadFromFile(const Device& device, const std::string& filepath)
    {
        this->device = device.getHandle();

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

        loadScenes(gltfModel);
        loadNodes(gltfModel);
        loadMeshes(device, gltfModel);
        loadMaterials(gltfModel);
        loadTextures(device, gltfModel);
    }


    void Model::loadScenes(tinygltf::Model& gltfModel)
    {
        for (auto& scene : gltfModel.scenes) {
            Scene sc;
            sc.nodes = scene.nodes;
        }
    }


    void Model::loadNodes(tinygltf::Model& gltfModel)
    {
        for (auto& node : gltfModel.nodes) {
            Node nd;
            nd.children = node.children;
            nd.mesh = node.mesh;

            glm::vec3 translation{ 0.0f };
            if (node.translation.size() == 3) {
                translation = glm::make_vec3(node.translation.data());
                nd.translation = translation;
            }
            glm::mat4 rotation{ 1.0f };
            if (node.rotation.size() == 4) {
                glm::quat q = glm::make_quat(node.rotation.data());
                nd.rotation = glm::mat4(q);
            }
            glm::vec3 scale{ 1.0f };
            if (node.scale.size() == 3) {
                scale = glm::make_vec3(node.scale.data());
                nd.scale = scale;
            }
            if (node.matrix.size() == 16) {
                nd.worldMatrix = glm::make_mat4x4(node.matrix.data());
            };

            nodes.push_back(nd);
        }
    }


    void Model::loadMeshes(const Device& device, tinygltf::Model& gltfModel)
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
                if (flipY) {
                    vert.pos = -vert.pos;
                    vert.normal = -vert.normal;
                }
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
            mesh.create(device, vertices, indices);
            mesh.material = gltfPrimitive.material;
            meshes.push_back(std::move(mesh));
        }
    }


    void Model::loadMaterials(tinygltf::Model& gltfModel)
    {
        for (auto& mat : gltfModel.materials) {
            Material material;

            // Base color
            if (mat.values.find("baseColorTexture") != mat.values.end()) {
                material.baseColorTexture = mat.values["baseColorTexture"].TextureIndex();
            }
            if (mat.values.find("baseColorFactor") != mat.values.end()) {
                material.baseColorFactor = glm::make_vec4(mat.values["baseColorFactor"].ColorFactor().data());
            }

            // Metallic / Roughness
            if (mat.values.find("metallicRoughnessTexture") != mat.values.end()) {
                material.metallicRoughnessTexture = mat.values["metallicRoughnessTexture"].TextureIndex();
            }
            if (mat.values.find("roughnessFactor") != mat.values.end()) {
                material.roughnessFactor = static_cast<float>(mat.values["roughnessFactor"].Factor());
            }
            if (mat.values.find("metallicFactor") != mat.values.end()) {
                material.metallicFactor = static_cast<float>(mat.values["metallicFactor"].Factor());
            }

            // Normal
            if (mat.additionalValues.find("normalTexture") != mat.additionalValues.end()) {
                material.normalTexture = mat.additionalValues["normalTexture"].TextureIndex();
            }

            // Emissive
            if (mat.additionalValues.find("emissiveTexture") != mat.additionalValues.end()) {
                material.emissiveTexture = mat.additionalValues["emissiveTexture"].TextureIndex();
            }

            // Occlusion
            if (mat.additionalValues.find("occlusionTexture") != mat.additionalValues.end()) {
                material.occlusionTexture = mat.additionalValues["occlusionTexture"].TextureIndex();
            }

            // Alpha
            if (mat.additionalValues.find("alphaMode") != mat.additionalValues.end()) {
                auto param = mat.additionalValues["alphaMode"];
                if (param.string_value == "BLEND") {
                    material.alphaMode = AlphaMode::Blend;
                }
                if (param.string_value == "MASK") {
                    material.alphaMode = AlphaMode::Mask;
                }
            }
            if (mat.additionalValues.find("alphaCutoff") != mat.additionalValues.end()) {
                material.alphaCutoff = static_cast<float>(mat.additionalValues["alphaCutoff"].Factor());
            }

            materials.push_back(material);
        }
    }


    void Model::loadTextures(const Device& device, tinygltf::Model& gltfModel)
    {
        for (auto& image : gltfModel.images) {
            Texture tex;

            if (image.component == 3) {
                throw std::runtime_error("3 component image is not supported"); // TODO support RGB
            }

            auto buffer = &image.image[0];
            tex.deviceSize = image.image.size();

            // Create image
            vk::Extent2D extent{ static_cast<uint32_t>(image.width), static_cast<uint32_t>(image.height) };
            vk::Format format{ vk::Format::eR8G8B8A8Unorm };
            tex.mipLevels = 1; // TODO support mipmap
            tex.image = std::make_unique<Image>(device, extent, format, vk::MemoryPropertyFlagBits::eDeviceLocal,
                                                vk::ImageAspectFlagBits::eColor);

            // Set image layout for transfer
            vk::UniqueCommandBuffer cmdBuf = device.createCommandBuffer();
            tex.image->transitionImageLayout(*cmdBuf, vk::ImageLayout::eTransferDstOptimal);

            // Copy from staging buffer
            using vkbu = vk::BufferUsageFlagBits;
            using vkmp = vk::MemoryPropertyFlagBits;
            Buffer stageBuf{ device, tex.deviceSize, vkbu::eTransferSrc, vkmp::eHostVisible | vkmp::eHostCoherent, buffer };
            tex.image->copyFrom(*cmdBuf, stageBuf);

            // Set image layout for shader
            tex.image->transitionImageLayout(*cmdBuf, vk::ImageLayout::eShaderReadOnlyOptimal);

            device.submitCommandBuffer(*cmdBuf);

            // Create sampler
            vk::SamplerCreateInfo samplerInfo{};
            samplerInfo.magFilter = vk::Filter::eLinear;
            samplerInfo.minFilter = vk::Filter::eLinear;
            samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
            samplerInfo.addressModeU = vk::SamplerAddressMode::eMirroredRepeat;
            samplerInfo.addressModeV = vk::SamplerAddressMode::eMirroredRepeat;
            samplerInfo.addressModeW = vk::SamplerAddressMode::eMirroredRepeat;
            samplerInfo.compareOp = vk::CompareOp::eNever;
            samplerInfo.borderColor = vk::BorderColor::eFloatOpaqueWhite;
            samplerInfo.maxAnisotropy = 1.0;
            samplerInfo.anisotropyEnable = false;
            samplerInfo.maxLod = (float)tex.mipLevels;
            tex.sampler = device.getHandle().createSamplerUnique(samplerInfo);

            textures.push_back(std::move(tex));
        }
    }

} // vkr
