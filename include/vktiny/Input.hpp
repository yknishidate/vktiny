#pragma once
#include <GLFW/glfw3.h>
#include <vktiny/Vulkan/Context.hpp>

namespace vkt
{
    class Input
    {
    public:
        Input() = default;
        Input(const Input&) = delete;
        Input(Input&&) = default;
        Input& operator = (const Input&) = delete;
        Input& operator = (Input&&) = default;

        void initialize(const Context& context)
        {
            this->window = context.getGLFWWindow();
        }

        void initialize(GLFWwindow* window)
        {
            this->window = window;
        }

        void prepare()
        {
            setInputCallbacks();
        }

        // setter
        void setOnKey(std::function<void(const int, const int, const int, const int)> onKey)
        {
            this->onKey = onKey;
        }
        void setOnCursorPosition(std::function<void(const double, const double)> onCursorPosition)
        {
            this->onCursorPosition = onCursorPosition;
        }
        void setOnMouseButton(std::function<void(const int, const int, const int)> onMouseButton)
        {
            this->onMouseButton = onMouseButton;
        }
        void setOnScroll(std::function<void(const double, const double)> onScroll)
        {
            this->onScroll = onScroll;
        }

        // callback
        void _keyCallback(const int key, const int scancode, const int action, const int mods)
        {
            onKey(key, scancode, action, mods);
        }
        void _cursorPositionCallback(const double xpos, const double ypos)
        {
            onCursorPosition(xpos, ypos);
        }
        void _mouseButtonCallback(const int button, const int action, const int mods)
        {
            onMouseButton(button, action, mods);
        }
        void _scrollCallback(const double xoffset, const double yoffset)
        {
            onScroll(xoffset, yoffset);
        }

    private:
        std::function<void(const int, const int, const int, const int)> onKey;
        std::function<void(const double, const double)> onCursorPosition;
        std::function<void(const int, const int, const int)> onMouseButton;
        std::function<void(const double, const double)> onScroll;

        void setInputCallbacks();

        GLFWwindow* window;
    };
}
