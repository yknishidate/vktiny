
#include <memory>

#include "vkbase.hpp"

class Application
{
public:
    void run()
    {
        window = std::make_unique<vkray::Window>("vkray", 800, 600);
        instance = std::make_unique<vkray::Instance>(*window, true);
        device = std::make_unique<vkray::Device>(*instance);

        //surface = window->createSurface(*instance);
        //physicalDevice = instance->pickSuitablePhysicalDevice();

        window->run();
    }

private:
    // vkray object
    std::unique_ptr<vkray::Window> window;
    std::unique_ptr<vkray::Instance> instance;
    std::unique_ptr<vkray::Device> device;

    //vk::UniqueSurfaceKHR surface;
    //vk::PhysicalDevice physicalDevice;
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
