#include "vktiny/vktiny.hpp"
#include <iostream>

class App
{
public:
    App()
    {
        //vkt::ContextCreateInfo contextInfo;
        //contextInfo.addDeviceExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

        context.initialize();
    }

    void run()
    {
        while (!context.shouldTerminate()) {
            context.pollEvents();
        }
    }

private:
    vkt::Context context;
};

int main()
{
    try {
        App app;
        app.run();
        std::cout << "OK!" << std::endl;
    } catch (std::exception exception) {
        std::cerr << exception.what() << std::endl;
    }
}
