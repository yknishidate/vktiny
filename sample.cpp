
#include <memory>

#include "vkray.hpp"

class Application
{
public:
    Application()
        : window{ "vkray", 800, 600 }
        , instance{ window, true }
        , device{ instance }
        , swapChain{ device }
    { }

    void run()
    {
        window.run();
    }

private:
    vkray::Window window;
    vkray::Instance instance;
    vkray::Device device;
    vkray::SwapChain swapChain;

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
