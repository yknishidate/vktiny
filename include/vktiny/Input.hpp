#pragma once
#include <GLFW/glfw3.h>
#include <vktiny/Vulkan/Context.hpp>

namespace vkt
{
    namespace InputState
    {
        enum
        {
            Release,
            Press,
        };
    }

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
            setInputCallbacks();
        }

        void initialize(GLFWwindow* window)
        {
            this->window = window;
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
            if (onKey) onKey(key, scancode, action, mods);
        }
        void _cursorPositionCallback(const double xpos, const double ypos)
        {
            if (onCursorPosition) onCursorPosition(xpos, ypos);
        }
        void _mouseButtonCallback(const int button, const int action, const int mods)
        {
            if (onMouseButton) onMouseButton(button, action, mods);
        }
        void _scrollCallback(const double xoffset, const double yoffset)
        {
            if (onScroll) onScroll(xoffset, yoffset);
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
