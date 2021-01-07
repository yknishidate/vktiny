
#include <stdexcept>
#include "vkray.hpp"

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

class Application
{
public:
    void run()
    {
        initVulkan();
        mainLoop();
    }

private:
    vkray::Context context;

    void initVulkan()
    {
        context.init(true, "SampleApp");
    }

    void mainLoop()
    {
        while (context.shouldStop()) {
            context.processEvents();
        }
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
