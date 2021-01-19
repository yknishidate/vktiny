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

#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

//#define STBI_NO_BMP
//#define STBI_NO_PSD
//#define STBI_NO_TGA
//#define STBI_NO_GIF
//#define STBI_NO_HDR
//#define STBI_NO_PIC
//#define STBI_NO_PNM
//#include <stb_image.h>

//#define TINYGLTF_NO_STB_IMAGE_WRITE
//#ifdef VK_USE_PLATFORM_ANDROID_KHR
//#define TINYGLTF_ANDROID_LOAD_FROM_ASSETS
//#endif
//#include <tiny_gltf.h>

namespace vkr
{
    class Window;

    class Instance;

    class Device;

    class Image;

    class Buffer;

    class VertexBuffer;

    class IndexBuffer;

    class DescriptorSets;

    class ShaderManager;


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


    class Window final
    {
    public:

        Window(const std::string& title, const uint32_t width, const uint32_t height,
            bool cursorDisabled = false, bool fullscreen = false, bool resizable = false);

        ~Window()
        {
            if (window != nullptr) {
                glfwDestroyWindow(window);
                window = nullptr;
            }

            glfwTerminate();
            glfwSetErrorCallback(nullptr);
        }

        Window(const Window&) = delete;

        Window(Window&&) = delete;

        Window& operator = (const Window&) = delete;

        Window& operator = (Window&&) = delete;

        const GLFWwindow* getHandle() const { return window; }

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

        std::function<void(int key, int scancode, int action, int mods)> onKey;

        std::function<void(double xpos, double ypos)> onCursorPosition;

        std::function<void(int button, int action, int mods)> onMouseButton;

        std::function<void(double xoffset, double yoffset)> onScroll;

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

        void createWindowSurface(VkInstance instance, VkSurfaceKHR& surface) const
        {
            if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
                throw std::runtime_error("failed to create window surface!");
            }
        }

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

        ~Instance() {}

        Instance(const Instance&) = delete;

        Instance(Instance&&) = delete;

        Instance& operator = (const Instance&) = delete;

        Instance& operator = (Instance&&) = delete;

        vk::Instance getHandle() const { return *instance; }

        const Window& getWindow() const { return window; }

        const std::vector<vk::ExtensionProperties>& getExtensionProperties() const { return extensionProperties; }

        const std::vector<vk::PhysicalDevice>& getPhysicalDevices() const { return physicalDevices; }

        const std::vector<const char*>& getValidationLayers() const { return validationLayers; }

        vk::UniqueSurfaceKHR createSurface() const
        {
            VkSurfaceKHR _surface;

            window.createWindowSurface(VkInstance(*instance), _surface);

            vk::ObjectDestroy<vk::Instance, VULKAN_HPP_DEFAULT_DISPATCHER_TYPE> _deleter(*instance);

            return vk::UniqueSurfaceKHR{ vk::SurfaceKHR(_surface), _deleter };
        }

        vk::PhysicalDevice pickSuitablePhysicalDevice() const
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

    private:

        void createDebugMessenger();

        static void checkVulkanMinimumVersion(uint32_t minVersion)
        {
            uint32_t version = vk::enumerateInstanceVersion();

            if (minVersion > version) {
                throw std::runtime_error("minimum required version not found");
            }
        }

        static void checkVulkanValidationLayerSupport(const std::vector<const char*>& validationLayers)
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

        vk::UniqueInstance instance;

        vk::UniqueDebugUtilsMessengerEXT messenger;

        const Window& window;

        std::vector<const char*> validationLayers;

        std::vector<vk::PhysicalDevice> physicalDevices;

        std::vector<vk::ExtensionProperties> extensionProperties;

        bool enableValidationLayers;
    };


    class Device final
    {
    public:
        explicit Device(const Instance& instance);
        ~Device() {}

        Device(const Device&) = delete;
        Device(Device&&) = delete;
        Device& operator = (const Device&) = delete;
        Device& operator = (Device&&) = delete;

        vk::Device getHandle() const { return *device; }

        vk::PhysicalDevice getPhysicalDevice() const { return physicalDevice; }
        vk::SurfaceKHR getSurface() const { return *surface; }
        const Instance& getInstance() const { return instance; }

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
        vk::UniqueCommandBuffer createCommandBuffer(vk::CommandBufferLevel level, bool begin,
            vk::CommandBufferUsageFlags usage = vk::CommandBufferUsageFlagBits::eOneTimeSubmit) const;
        void submitCommandBuffer(vk::CommandBuffer& commandBuffer) const;
        std::unique_ptr<VertexBuffer> createVertexBuffer(std::vector<Vertex>& vertices, bool onDevice) const;
        std::unique_ptr<IndexBuffer> createIndexBuffer(std::vector<uint32_t>& indices, bool onDevice) const;
        vk::UniquePipeline createRayTracingPipeline(const DescriptorSets& descSets, const ShaderManager& shaderManager, uint32_t maxRecursionDepth);

    private:

        void checkRequiredExtensions(vk::PhysicalDevice physicalDevice) const
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

        const Instance& instance;

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
        explicit SwapChain(const Device& device);
        ~SwapChain()
        {
            for (size_t i = 0; i < maxFramesInFlight; i++) {
                device.getHandle().destroyFence(inFlightFences[i]);
            }
        }

        SwapChain(const SwapChain&) = delete;
        SwapChain(SwapChain&&) = delete;
        SwapChain& operator = (const SwapChain&) = delete;
        SwapChain& operator = (SwapChain&&) = delete;

        vk::PhysicalDevice getPhysicalDevice() const { return physicalDevice; }
        const Device& getDevice() const { return device; }
        uint32_t getMinImageCount() const { return minImageCount; }
        const std::vector<vk::Image>& getImages() const { return images; }
        const std::vector<vk::UniqueImageView>& getImageViews() const { return imageViews; }
        const vk::Extent2D& getExtent() const { return extent; }
        vk::Format getFormat() const { return format; }
        vk::PresentModeKHR getPresentMode() const { return presentMode; }

        std::unique_ptr<Image> createStorageImage() const;

        void initDrawCommandBuffers(vk::Pipeline pipeline, const DescriptorSets& descSets, const ShaderManager& shaderManager, vkr::Image& storageImage);

        void draw()
        {
            device.getHandle().waitForFences(inFlightFences[currentFrame], true, std::numeric_limits<uint64_t>::max());

            auto result = device.getHandle().acquireNextImageKHR(
                swapChain.get(),                             // swapchain
                std::numeric_limits<uint64_t>::max(),        // timeout
                imageAvailableSemaphores[currentFrame].get() // semaphore
            );
            uint32_t imageIndex;
            if (result.result == vk::Result::eSuccess) {
                imageIndex = result.value;
            } else {
                throw std::runtime_error("failed to acquire next image!");
            }

            if (imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
                device.getHandle().waitForFences(imagesInFlight[imageIndex], true, std::numeric_limits<uint64_t>::max());
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

            device.getGraphicsQueue().presentKHR(
                vk::PresentInfoKHR{}
                .setWaitSemaphores(renderFinishedSemaphores[currentFrame].get())
                .setSwapchains(swapChain.get())
                .setImageIndices(imageIndex)
            );

            currentFrame = (currentFrame + 1) % maxFramesInFlight;
        }

    private:

        struct SupportDetails
        {
            vk::SurfaceCapabilitiesKHR capabilities{};
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

        uint32_t minImageCount;
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
        Image(const Image&) = delete;
        Image& operator = (const Image&) = delete;
        Image& operator = (Image&& other) = delete;

        Image(const Device& device, vk::Extent2D extent, vk::Format format);
        Image(const Device& device, vk::Extent2D extent, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage);
        Image(Image&& other) noexcept;
        ~Image() {}

        const class Device& getDevice() const { return device; }
        vk::Extent2D getExtent() const { return extent; }
        vk::Format getFormat() const { return format; }
        vk::ImageView getView() const { return *view; }
        vk::Image getHandle() const { return *image; }

        void allocateMemory(vk::MemoryPropertyFlags properties);
        void addImageView(vk::ImageAspectFlags aspectFlags);
        vk::MemoryRequirements getMemoryRequirements() const
        {
            return device.getHandle().getImageMemoryRequirements(*image);
        }

        //void copyFrom(const Buffer& buffer);

    private:

        const Device& device;

        vk::UniqueImage image;
        vk::UniqueImageView view;
        vk::UniqueDeviceMemory memory;

        const vk::Extent2D extent;
        const vk::Format format;
        vk::ImageLayout imageLayout;

    };


    class Buffer
    {
    public:
        Buffer(const Device& device, vk::DeviceSize size, vk::BufferUsageFlags usage);
        Buffer(const Device& device, vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, void* data = nullptr);
        ~Buffer() {}

        Buffer(const Buffer&) = delete;
        Buffer(Buffer&&) = delete;
        Buffer& operator = (const Buffer&) = delete;
        Buffer& operator = (Buffer&&) = delete;

        const Device& getDevice() const { return device; }
        vk::DeviceSize getSize() const { return size; }
        vk::Buffer getHandle() const { return *buffer; }

        uint64_t getDeviceAddress()
        {
            vk::BufferDeviceAddressInfoKHR bufferDeviceAI{ *buffer };
            return device.getHandle().getBufferAddressKHR(&bufferDeviceAI);
        }

        vk::MemoryRequirements getMemoryRequirements() const
        {
            return device.getHandle().getBufferMemoryRequirements(*buffer);
        }

        void copyFrom(const Buffer& src);

    private:
        const Device& device;

        vk::UniqueBuffer buffer;
        vk::UniqueDeviceMemory memory;
        vk::DeviceSize size;
    };


    class VertexBuffer final : public Buffer
    {
    public:
        VertexBuffer(const Device& device, vk::DeviceSize size, vk::BufferUsageFlags usage,
            vk::MemoryPropertyFlags properties, std::vector<Vertex>& vertices)
            : Buffer(device, size, usage, properties, vertices.data())
        {
            verticesCount = static_cast<uint32_t>(vertices.size());
        }

    private:
        uint32_t verticesCount;
    };


    class IndexBuffer final : public Buffer
    {
    public:
        IndexBuffer(const Device& device, vk::DeviceSize size, vk::BufferUsageFlags usage,
            vk::MemoryPropertyFlags properties, std::vector<uint32_t>& indices)
            : Buffer(device, size, usage, properties, indices.data())
        {
            indicesCount = static_cast<uint32_t>(indices.size());
        }

    private:
        uint32_t indicesCount;
    };


    class DescriptorSetBindings final
    {
    public:
        DescriptorSetBindings(const Device& device) : device(device) {}

        DescriptorSetBindings(const DescriptorSetBindings&) = delete;
        DescriptorSetBindings(DescriptorSetBindings&&) = delete;
        DescriptorSetBindings& operator = (const DescriptorSetBindings&) = delete;
        DescriptorSetBindings& operator = (DescriptorSetBindings&&) = delete;

        void addBindging(uint32_t binding, vk::DescriptorType type, uint32_t count,
            vk::ShaderStageFlags stageFlags, const vk::Sampler* pImmutableSampler = nullptr);

        vk::UniqueDescriptorSetLayout createLayout(vk::DescriptorSetLayoutCreateFlags flags = {}) const;

        void addRequiredPoolSizes(std::vector<vk::DescriptorPoolSize>& poolSizes) const;

        vk::WriteDescriptorSet makeWrite(vk::DescriptorSet dstSet, uint32_t dstBinding,
            const vk::DescriptorImageInfo* pImageInfo, uint32_t arrayElement = 0) const;

        vk::WriteDescriptorSet makeWrite(vk::DescriptorSet dstSet, uint32_t dstBinding,
            const vk::DescriptorBufferInfo* pBufferInfo, uint32_t arrayElement = 0) const;

        vk::WriteDescriptorSet makeWrite(vk::DescriptorSet dstSet, uint32_t dstBinding,
            const vk::WriteDescriptorSetAccelerationStructureKHR* pASInfo, uint32_t arrayElement = 0) const;

    private:
        const Device& device;
        std::vector<vk::DescriptorSetLayoutBinding> bindings;
    };


    class DescriptorSets final
    {
    public:
        DescriptorSets(const Device& device, uint32_t numSets = 1);

        DescriptorSets(const DescriptorSets&) = delete;
        DescriptorSets(DescriptorSets&&) = delete;
        DescriptorSets& operator = (const DescriptorSets&) = delete;
        DescriptorSets& operator = (DescriptorSets&&) = delete;

        vk::PipelineLayout getPipelineLayout() const { return *pipeLayout; }
        std::vector<vk::DescriptorSet> getDescriptorSets() const
        {
            std::vector<vk::DescriptorSet> rawDescSets;
            for (auto& descSet : descSets) {
                rawDescSets.push_back(*descSet);
            }
            return rawDescSets;
        }
        std::vector<vk::DescriptorSetLayout> getDescriptorSetLayouts() const
        {
            std::vector<vk::DescriptorSetLayout> rawDescSetLayouts;
            for (auto& descSetLayout : descSetLayouts) {
                rawDescSetLayouts.push_back(*descSetLayout);
            }
            return rawDescSetLayouts;
        }

        void addBindging(uint32_t setIndex, uint32_t binding, vk::DescriptorType type, uint32_t count,
            vk::ShaderStageFlags stageFlags, const vk::Sampler* pImmutableSampler = nullptr);

        void initPipelineLayout();
        vk::PipelineLayout createPipelineLayout();

        void addWriteInfo(uint32_t setIndex, uint32_t binding, const vk::WriteDescriptorSetAccelerationStructureKHR& writeInfo)
        {
            writeDescSets.push_back(bindingsArray[setIndex]->makeWrite(*descSets[setIndex], binding, &writeInfo));
        }

        void addWriteInfo(uint32_t setIndex, uint32_t binding, const vk::DescriptorImageInfo& writeInfo)
        {
            writeDescSets.push_back(bindingsArray[setIndex]->makeWrite(*descSets[setIndex], binding, &writeInfo));
        }

        void addWriteInfo(uint32_t setIndex, uint32_t binding, const vk::DescriptorBufferInfo& writeInfo)
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

        ShaderManager(const ShaderManager&) = delete;
        ShaderManager(ShaderManager&&) = delete;
        ShaderManager& operator = (const ShaderManager&) = delete;
        ShaderManager& operator = (ShaderManager&&) = delete;

        auto getStages() const { return stages; }
        auto getRayTracingGroups() const { return rtGroups; }
        auto getRaygenRegion() const { return raygenRegion; }
        auto getMissRegion() const { return missRegion; }
        auto getHitRegion() const { return hitRegion; }

        void addShader(const std::string& filename, vk::ShaderStageFlagBits stage, const char* pName,
            vk::RayTracingShaderGroupTypeKHR groupType);

        void addShader(uint32_t moduleIndex, vk::ShaderStageFlagBits stage, const std::string& pName,
            vk::RayTracingShaderGroupTypeKHR groupType);

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
        AccelerationStructure(const Device& device) : device(device) {}

        AccelerationStructure(const AccelerationStructure&) = delete;
        AccelerationStructure& operator = (const AccelerationStructure&) = delete;
        AccelerationStructure& operator = (AccelerationStructure&&) = delete;

        AccelerationStructure(AccelerationStructure&& other) noexcept;

        const Device& getDevice() const { return device; }
        const uint64_t getDeviceAddress() const { return deviceAddress; }

        vk::AccelerationStructureKHR& getHandle() { return *accelerationStructure; }

    protected:
        void build(vk::AccelerationStructureGeometryKHR& geometry, const vk::AccelerationStructureTypeKHR& asType, uint32_t primitiveCount);

        void createBuffer(vk::AccelerationStructureBuildSizesInfoKHR buildSizesInfo);

        Buffer createScratchBuffer(vk::DeviceSize size);

        const Device& device;

        vk::UniqueAccelerationStructureKHR accelerationStructure;

        std::unique_ptr<Buffer> buffer;

        uint64_t deviceAddress;
    };


    class BottomLevelAccelerationStructure final : public AccelerationStructure
    {
    public:
        BottomLevelAccelerationStructure(const Device& device, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);
        //BottomLevelAccelerationStructure(const Device& device, const Buffer& vertexBuffer, const Buffer& indexBuffer);

        BottomLevelAccelerationStructure(const BottomLevelAccelerationStructure&) = delete;
        BottomLevelAccelerationStructure& operator = (const BottomLevelAccelerationStructure&) = delete;
        BottomLevelAccelerationStructure& operator = (BottomLevelAccelerationStructure&&) = delete;

        BottomLevelAccelerationStructure(BottomLevelAccelerationStructure&& other) noexcept;
    };


    class TopLevelAccelerationStructure final : public AccelerationStructure
    {
    public:
        // TODO: vector input
        //TopLevelAccelerationStructure(const Device& device, std::vector<AccelerationStructureInstance>& instances);
        TopLevelAccelerationStructure(const Device& device, BottomLevelAccelerationStructure& blas, AccelerationStructureInstance& instance);

        TopLevelAccelerationStructure(const BottomLevelAccelerationStructure&) = delete;
        TopLevelAccelerationStructure& operator = (const BottomLevelAccelerationStructure&) = delete;
        TopLevelAccelerationStructure& operator = (BottomLevelAccelerationStructure&&) = delete;

        TopLevelAccelerationStructure(BottomLevelAccelerationStructure&& other) noexcept;

    };


    namespace
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

            // コマンドバッファにバリアを積む
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

    } // namespace


    ////////////////////
    // implementation //
    ////////////////////


    // Window
    Window::Window(const std::string& title, const uint32_t width, const uint32_t height, bool cursorDisabled, bool fullscreen, bool resizable)
        : title(title), width(width), height(height), cursorDisabled(cursorDisabled), fullscreen(fullscreen), resizable(resizable)
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

        validationLayers = enableValidationLayers ? std::vector<const char*>{"VK_LAYER_KHRONOS_validation"} : std::vector<const char*>();

        const uint32_t version = VK_API_VERSION_1_2;
        checkVulkanMinimumVersion(version);

        auto extensions = window.getRequiredInstanceExtensions();

        checkVulkanValidationLayerSupport(validationLayers);
        if (enableValidationLayers) {
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

    void Instance::createDebugMessenger()
    {
        vk::DebugUtilsMessageSeverityFlagsEXT severityFlags{
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning
            | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError };

        vk::DebugUtilsMessageTypeFlagsEXT messageTypeFlags{
            vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral
            | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance
            | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation };

        messenger = instance->createDebugUtilsMessengerEXTUnique({ {}, severityFlags, messageTypeFlags, &debugUtilsMessengerCallback });
    }

    // Device
    Device::Device(const Instance& instance)
        : instance(instance)
    {
        surface = instance.createSurface();
        physicalDevice = instance.pickSuitablePhysicalDevice();

        checkRequiredExtensions(physicalDevice);

        const auto queueFamilies = physicalDevice.getQueueFamilyProperties();

        // Find the graphics queue
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

        graphicsQueue.submit(vk::SubmitInfo{}.setCommandBuffers(commandBuffer), fence.get());

        device->waitForFences(fence.get(), true, std::numeric_limits<uint64_t>::max());
    }

    std::unique_ptr<VertexBuffer> Device::createVertexBuffer(std::vector<Vertex>& vertices, bool onDevice) const
    {
        auto vertexBufferSize = vertices.size() * sizeof(Vertex);

        vk::BufferUsageFlags bufferUsage;
        vk::MemoryPropertyFlags memoryProperty;

        // TODO: デバイスに送れるようにする
        //if (onDevice) {
        //    bufferUsage = vk::BufferUsageFlagBits::eVertexBuffer
        //                | vk::BufferUsageFlagBits::eStorageBuffer
        //                | vk::BufferUsageFlagBits::eTransferDst
        //                | vk::BufferUsageFlagBits::eShaderDeviceAddress;
        //} else {
        //    bufferUsage = vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR
        //                | vk::BufferUsageFlagBits::eTransferSrc;
        //}

        bufferUsage = vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR
            | vk::BufferUsageFlagBits::eStorageBuffer
            //| vk::BufferUsageFlagBits::eVertexBuffer
            | vk::BufferUsageFlagBits::eShaderDeviceAddress;
        memoryProperty = vk::MemoryPropertyFlagBits::eHostVisible
            | vk::MemoryPropertyFlagBits::eHostCoherent;

        return std::make_unique<VertexBuffer>(*this, vertexBufferSize, bufferUsage, memoryProperty, vertices);
    }

    std::unique_ptr<IndexBuffer> Device::createIndexBuffer(std::vector<uint32_t>& indices, bool onDevice) const
    {
        auto indexBufferSize = indices.size() * sizeof(uint32_t);

        vk::BufferUsageFlags bufferUsage;
        vk::MemoryPropertyFlags memoryProperty;

        // TODO: デバイスに送れるようにする

        bufferUsage = vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR
            | vk::BufferUsageFlagBits::eStorageBuffer
            //| vk::BufferUsageFlagBits::eIndexBuffer
            | vk::BufferUsageFlagBits::eShaderDeviceAddress;
        memoryProperty = vk::MemoryPropertyFlagBits::eHostVisible
            | vk::MemoryPropertyFlagBits::eHostCoherent;

        return std::make_unique<IndexBuffer>(*this, indexBufferSize, bufferUsage, memoryProperty, indices);
    }

    vk::UniquePipeline Device::createRayTracingPipeline(const DescriptorSets& descSets, const ShaderManager& shaderManager, uint32_t maxRecursionDepth)
    {
        auto result = device->createRayTracingPipelineKHRUnique(nullptr, nullptr,
            vk::RayTracingPipelineCreateInfoKHR{}
            .setStages(shaderManager.getStages())
            .setGroups(shaderManager.getRayTracingGroups())
            .setMaxPipelineRayRecursionDepth(maxRecursionDepth)
            .setLayout(descSets.getPipelineLayout())
        );
        if (result.result == vk::Result::eSuccess) {
            return std::move(result.value);
        } else {
            throw std::runtime_error("failed to create ray tracing pipeline.");
        }
    }

    // SwapChain
    SwapChain::SwapChain(const Device& device)
        : device(device)
        , physicalDevice(device.getPhysicalDevice())
    {
        const auto details = querySwapChainSupport(device.getPhysicalDevice(), device.getSurface());
        if (details.formats.empty() || details.presentModes.empty()) {
            throw std::runtime_error("empty swap chain support");
        }

        const auto& surface = device.getSurface();
        const auto& window = device.getInstance().getWindow();

        const auto surfaceFormat = chooseSwapSurfaceFormat(details.formats);
        const auto actualPresentMode = chooseSwapPresentMode(details.presentModes);
        const auto swapExtent = chooseSwapExtent(window, details.capabilities);
        const auto imageCount = chooseImageCount(details.capabilities);

        // Create swap chain
        vk::SwapchainCreateInfoKHR createInfo{};
        createInfo.surface = surface;
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = swapExtent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst;
        createInfo.imageSharingMode = vk::SharingMode::eExclusive;
        createInfo.preTransform = details.capabilities.currentTransform;
        createInfo.presentMode = actualPresentMode;
        createInfo.clipped = true;

        if (device.getGraphicsFamilyIndex() != device.getPresentFamilyIndex()) {
            uint32_t queueFamilyIndices[] = { device.getGraphicsFamilyIndex(), device.getPresentFamilyIndex() };
            createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        }

        swapChain = device.getHandle().createSwapchainKHRUnique(createInfo);

        minImageCount = imageCount;
        presentMode = actualPresentMode;
        format = surfaceFormat.format;
        extent = swapExtent;
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
        auto image = std::make_unique<Image>(device, extent, format, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eTransferSrc);
        image->allocateMemory(vk::MemoryPropertyFlagBits::eDeviceLocal);
        image->addImageView(vk::ImageAspectFlagBits::eColor);

        auto commandBuffer = device.createCommandBuffer(vk::CommandBufferLevel::ePrimary, true);
        transitionImageLayout(commandBuffer.get(), image->getHandle(), vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral);
        device.submitCommandBuffer(commandBuffer.get());

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
                {},            // callableShaderBindingTable
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

    // Image
    Image::Image(const class Device& device, const vk::Extent2D extent, const vk::Format format)
        : Image(device, extent, format, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled)
    {
    }

    Image::Image(const class Device& device, const vk::Extent2D extent, const vk::Format format,
        const vk::ImageTiling tiling, const vk::ImageUsageFlags usage)
        : device(device), extent(extent), format(format), imageLayout(vk::ImageLayout::eUndefined)
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
        imageInfo.initialLayout = imageLayout;
        imageInfo.usage = usage;
        imageInfo.sharingMode = vk::SharingMode::eExclusive;
        imageInfo.samples = vk::SampleCountFlagBits::e1;

        image = device.getHandle().createImageUnique(imageInfo);
    }

    Image::Image(Image&& other) noexcept
        : device(other.device), extent(other.extent), format(other.format), imageLayout(other.imageLayout)
        , image(std::move(other.image)), view(std::move(other.view))
    {
        other.image.release();
        other.view.release();
    }

    void Image::allocateMemory(const vk::MemoryPropertyFlags properties)
    {
        const auto requirements = device.getHandle().getImageMemoryRequirements(*image);
        auto memoryTypeIndex = device.findMemoryType(requirements.memoryTypeBits, properties);
        memory = device.getHandle().allocateMemoryUnique({ requirements.size, memoryTypeIndex });

        device.getHandle().bindImageMemory(*image, *memory, 0);
    }

    void Image::addImageView(vk::ImageAspectFlags aspectFlags)
    {
        vk::ImageViewCreateInfo createInfo{ {}, *image, vk::ImageViewType::e2D, format };
        createInfo.subresourceRange = { aspectFlags , 0, 1, 0, 1 };
        view = device.getHandle().createImageViewUnique(createInfo);
    }

    // Buffer
    Buffer::Buffer(const Device& device, vk::DeviceSize size, vk::BufferUsageFlags usage)
        : device(device), size(size)
    {
        buffer = device.getHandle().createBufferUnique({ {}, size, usage, vk::SharingMode::eExclusive });
    }

    Buffer::Buffer(const Device& device, vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, void* data /*= nullptr*/)
        : device(device), size(size)
    {
        buffer = device.getHandle().createBufferUnique({ {}, size, usage, vk::SharingMode::eExclusive });

        const auto requirements = device.getHandle().getBufferMemoryRequirements(*buffer);

        vk::MemoryAllocateFlagsInfo flagsInfo{};
        if (usage & vk::BufferUsageFlagBits::eShaderDeviceAddress) {
            flagsInfo.flags = vk::MemoryAllocateFlagBits::eDeviceAddress;
        }

        vk::MemoryAllocateInfo allocInfo{};
        allocInfo.allocationSize = requirements.size;
        allocInfo.memoryTypeIndex = device.findMemoryType(requirements.memoryTypeBits, properties);
        allocInfo.pNext = &flagsInfo;
        memory = device.getHandle().allocateMemoryUnique(allocInfo);

        device.getHandle().bindBufferMemory(*buffer, *memory, 0);

        if (data) {
            void* dataPtr = device.getHandle().mapMemory(*memory, 0, size);
            memcpy(dataPtr, data, static_cast<size_t>(size));
            device.getHandle().unmapMemory(*memory);
        }
    }

    void Buffer::copyFrom(const Buffer& src)
    {
        auto commandBuffer = device.createCommandBuffer(vk::CommandBufferLevel::ePrimary, true);

        vk::BufferCopy copyRegion{};
        copyRegion.srcOffset = 0;
        copyRegion.dstOffset = 0;
        copyRegion.size = src.getSize();

        commandBuffer->copyBuffer(src.getHandle(), *buffer, copyRegion);

        device.submitCommandBuffer(*commandBuffer);
    }


    // DescriptorSetBindings
    void DescriptorSetBindings::addBindging(uint32_t binding, vk::DescriptorType type, uint32_t count,
        vk::ShaderStageFlags stageFlags, const vk::Sampler* pImmutableSampler/*= nullptr*/)
    {
        bindings.push_back({ binding, type, count, stageFlags, pImmutableSampler });
    }

    vk::UniqueDescriptorSetLayout DescriptorSetBindings::createLayout(vk::DescriptorSetLayoutCreateFlags flags) const
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

        for (int i = 0; i < numSets; i++) {
            bindingsArray.push_back(std::make_unique<DescriptorSetBindings>(device));
        }
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
            descSetLayouts.push_back(bindings->createLayout());
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
        const vk::BufferUsageFlags usage = vkbu::eShaderBindingTableKHR | vkbu::eTransferSrc | vkbu::eShaderDeviceAddress;
        const vk::MemoryPropertyFlags memoryProperty = vkmp::eHostVisible | vkmp::eHostCoherent;

        // Get shader group handles
        std::vector<uint8_t> shaderHandleStorage(sbtSize);
        auto result = device.getHandle().getRayTracingShaderGroupHandlesKHR(pipeline, 0, groupCount, static_cast<size_t>(sbtSize), shaderHandleStorage.data());
        if (result != vk::Result::eSuccess) {
            throw std::runtime_error("failed to get ray tracing shader group handles.");
        }

        // Create SBT Buffers
        raygenShaderBindingTable = std::make_unique<Buffer>(device, handleSize, usage, memoryProperty,
            shaderHandleStorage.data() + raygenOffset * handleSizeAligned);
        missShaderBindingTable = std::make_unique<Buffer>(device, handleSize, usage, memoryProperty,
            shaderHandleStorage.data() + missOffset * handleSizeAligned);
        hitShaderBindingTable = std::make_unique<Buffer>(device, handleSize, usage, memoryProperty,
            shaderHandleStorage.data() + hitOffset * handleSizeAligned);

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
    void AccelerationStructure::build(vk::AccelerationStructureGeometryKHR& geometry,
        const vk::AccelerationStructureTypeKHR& asType, uint32_t primitiveCount)
    {
        vk::AccelerationStructureBuildGeometryInfoKHR buildGeometryInfo{};
        buildGeometryInfo.setType(asType);
        buildGeometryInfo.setFlags(vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace);
        buildGeometryInfo.setGeometries(geometry);

        auto buildSizesInfo = device.getHandle().getAccelerationStructureBuildSizesKHR(
            vk::AccelerationStructureBuildTypeKHR::eDevice, buildGeometryInfo, primitiveCount);

        createBuffer(buildSizesInfo);

        accelerationStructure = device.getHandle().createAccelerationStructureKHRUnique(
            vk::AccelerationStructureCreateInfoKHR{}
            .setBuffer(buffer->getHandle())
            .setSize(buildSizesInfo.accelerationStructureSize)
            .setType(asType)
        );

        Buffer scratchBuffer{ device, buildSizesInfo.buildScratchSize,
            vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress,
            vk::MemoryPropertyFlagBits::eDeviceLocal };

        vk::AccelerationStructureBuildGeometryInfoKHR accelerationBuildGeometryInfo{};
        accelerationBuildGeometryInfo.setType(asType);
        accelerationBuildGeometryInfo.setFlags(vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace);
        accelerationBuildGeometryInfo.setDstAccelerationStructure(*accelerationStructure);
        accelerationBuildGeometryInfo.setGeometries(geometry);
        accelerationBuildGeometryInfo.setScratchData(scratchBuffer.getDeviceAddress());

        vk::AccelerationStructureBuildRangeInfoKHR accelerationStructureBuildRangeInfo{};
        accelerationStructureBuildRangeInfo
            .setPrimitiveCount(primitiveCount) // TODO: primitiveCount ?
            .setPrimitiveOffset(0)
            .setFirstVertex(0)
            .setTransformOffset(0);

        auto commandBuffer = device.createCommandBuffer(vk::CommandBufferLevel::ePrimary, true);
        commandBuffer->buildAccelerationStructuresKHR(accelerationBuildGeometryInfo, &accelerationStructureBuildRangeInfo);
        device.submitCommandBuffer(*commandBuffer);

        deviceAddress = device.getHandle().getAccelerationStructureAddressKHR({ *accelerationStructure });
    }

    void AccelerationStructure::createBuffer(vk::AccelerationStructureBuildSizesInfoKHR buildSizesInfo)
    {
        auto size = buildSizesInfo.accelerationStructureSize;
        auto usage = vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR | vk::BufferUsageFlagBits::eShaderDeviceAddress;
        // TODO: onDevice
        buffer = std::make_unique<Buffer>(device, size, usage, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
    }

    // BottomLevelAccelerationStructure
    BottomLevelAccelerationStructure::BottomLevelAccelerationStructure(const Device& device, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices)
        : AccelerationStructure(device)
    {
        auto vertexBuffer = device.createVertexBuffer(vertices, false);
        auto indexBuffer = device.createIndexBuffer(indices, false);

        vk::AccelerationStructureGeometryTrianglesDataKHR triangleData{};
        triangleData.setVertexFormat(vk::Format::eR32G32B32Sfloat);
        triangleData.setVertexData(vertexBuffer->getDeviceAddress());
        triangleData.setVertexStride(sizeof(Vertex));
        triangleData.setMaxVertex(static_cast<uint32_t>(vertices.size()));
        triangleData.setIndexType(vk::IndexType::eUint32);
        triangleData.setIndexData(indexBuffer->getDeviceAddress());

        vk::AccelerationStructureGeometryKHR geometry{};
        geometry.setGeometryType(vk::GeometryTypeKHR::eTriangles);
        geometry.setGeometry({ triangleData });
        geometry.setFlags(vk::GeometryFlagBitsKHR::eOpaque);

        uint32_t triangleCount = static_cast<uint32_t>(indices.size() / 3);
        build(geometry, vk::AccelerationStructureTypeKHR::eBottomLevel, triangleCount);

    }

    //BottomLevelAccelerationStructure::BottomLevelAccelerationStructure(const Device& device, Buffer& vertexBuffer, Buffer& indexBuffer)
    //    : AccelerationStructure(device)
    //{
    //    vk::AccelerationStructureGeometryTrianglesDataKHR triangleData{};
    //    triangleData
    //        .setVertexFormat(vk::Format::eR32G32B32Sfloat)
    //        .setVertexData(vertexBuffer.getDeviceAddress())
    //        .setVertexStride(sizeof(Vertex))
    //        .setMaxVertex(vertices.size())
    //        .setIndexType(vk::IndexType::eUint32)
    //        .setIndexData(indexBuffer.getDeviceAddress());
    //
    //    vk::AccelerationStructureGeometryKHR geometry{};
    //    geometry
    //        .setGeometryType(vk::GeometryTypeKHR::eTriangles)
    //        .setGeometry({ triangleData })
    //        .setFlags(vk::GeometryFlagBitsKHR::eOpaque);
    //
    //    build(geometry, vk::AccelerationStructureTypeKHR::eBottomLevel, 1);
    //}

    // TopLevelAccelerationStructure
    // TODO: これは実験用にしておく
    TopLevelAccelerationStructure::TopLevelAccelerationStructure(const Device& device, BottomLevelAccelerationStructure& blas, AccelerationStructureInstance& instance)
        : AccelerationStructure(device)
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
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, &asInstance };

        vk::AccelerationStructureGeometryInstancesDataKHR instancesData{};
        instancesData.setArrayOfPointers(false);
        instancesData.setData(instancesBuffer.getDeviceAddress());

        vk::AccelerationStructureGeometryKHR geometry{};
        geometry.setGeometryType(vk::GeometryTypeKHR::eInstances);
        geometry.setGeometry({ instancesData });
        geometry.setFlags(vk::GeometryFlagBitsKHR::eOpaque);

        uint32_t instanceCount = 1;
        build(geometry, vk::AccelerationStructureTypeKHR::eTopLevel, instanceCount);
    }


} // vkr
