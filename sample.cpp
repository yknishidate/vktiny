
#include <memory>

#include "vkbase.hpp"


const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

class Application
{
public:
    void run()
    {
        window = std::make_unique<vkray::Window>("vkray", WIDTH, HEIGHT);
        instance = std::make_unique<vkray::Instance>(*window, true);
        window->run();
    }

private:
    std::unique_ptr<vkray::Window> window;
    std::unique_ptr<vkray::Instance> instance;
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
