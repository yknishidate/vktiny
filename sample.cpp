
#include <memory>

#include "vkray.hpp"

class Application
{
public:

    void run()
    {
        window = std::make_unique<vkr::Window>("vkray", 800, 600);
        instance = std::make_unique<vkr::Instance>(*window, true);
        device = std::make_unique<vkr::Device>(*instance);
        swapChain = std::make_unique<vkr::SwapChain>(*device);

        outputImage = swapChain->createOutputImage();

        std::vector<vkr::Vertex> vertices;
        vertices.push_back(vkr::Vertex{ {1.0f, 1.0f, 0.0f} });
        vertices.push_back(vkr::Vertex{ {-1.0f, 1.0f, 0.0f} });
        vertices.push_back(vkr::Vertex{ {0.0f, -1.0f, 0.0f} });
        std::vector<uint32_t> indices = { 0, 1, 2 };

        blas = std::make_unique<vkr::BottomLevelAccelerationStructure>(*device, vertices, indices);

        window->run();
    }

private:

    std::unique_ptr<vkr::Window> window;
    std::unique_ptr<vkr::Instance> instance;
    std::unique_ptr<vkr::Device> device;
    std::unique_ptr<vkr::SwapChain> swapChain;
    std::unique_ptr<vkr::Image> outputImage;
    std::unique_ptr<vkr::BottomLevelAccelerationStructure> blas;

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
