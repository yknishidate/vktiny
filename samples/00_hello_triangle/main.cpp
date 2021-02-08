
#define QUICK_VKRAY_IMPLEMENTATION
#include "../../vkray.hpp"

using vkss = vk::ShaderStageFlagBits;
using vkdt = vk::DescriptorType;
using vksgt = vk::RayTracingShaderGroupTypeKHR;

constexpr float WIDTH = 1280;
constexpr float HEIGHT = 720;

class Application
{
public:

    Application() = default;

    ~Application()
    {
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    void run()
    {
        initWindow();
        initVulkan();
        mainLoop();
    }

private:

    GLFWwindow* window;

    std::unique_ptr<vkr::Instance> instance;

    vk::UniqueSurfaceKHR surface;

    std::unique_ptr<vkr::Device> device;

    void initWindow()
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        window = glfwCreateWindow(WIDTH, HEIGHT, "quick vkray", nullptr, nullptr);
    }

    void initVulkan()
    {
        createInstance();
        createSurface();
        device = std::make_unique<vkr::Device>(*instance, *surface);

    }

    void createInstance()
    {
        // Get GLFW extensions
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

#ifdef NDEBUG
        const bool enableValidationLayers = false;
#else
        const bool enableValidationLayers = true;
#endif

        vk::ApplicationInfo appInfo;
        appInfo.setPApplicationName("quick vkray");
        appInfo.setApiVersion(VK_API_VERSION_1_2);

        instance = std::make_unique<vkr::Instance>(appInfo, enableValidationLayers, extensions);
    }

    void createSurface()
    {
        VkSurfaceKHR _surface;
        if (glfwCreateWindowSurface(VkInstance(instance->getHandle()), window, nullptr, &_surface) != VK_SUCCESS) {
            throw std::runtime_error("failed to create window surface!");
        }
        vk::ObjectDestroy<vk::Instance, VULKAN_HPP_DEFAULT_DISPATCHER_TYPE> _deleter(instance->getHandle());
        surface = vk::UniqueSurfaceKHR(vk::SurfaceKHR(_surface), _deleter);
    }

    void mainLoop()
    {
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
        }
    }

    void cleanup()
    {
        glfwDestroyWindow(window);

        glfwTerminate();
    }
};

int main()
{
    Application app;

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
