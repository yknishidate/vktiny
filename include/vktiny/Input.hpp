#pragma once
#include <GLFW/glfw3.h>
#include <functional>

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

        void initialize(GLFWwindow* window)
        {
            this->window = window;
            setInputCallbacks();
        }

        void reset()
        {
            xoffset = 0.0;
            yoffset = 0.0;
            scroll = 0.0;
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
            xlast = this->xpos;
            ylast = this->ypos;
            this->xpos = xpos;
            this->ypos = ypos;
            xoffset = xpos - xlast;
            yoffset = ypos - ylast;
            if (onCursorPosition) onCursorPosition(xpos, ypos);
        }
        void _mouseButtonCallback(const int button, const int action, const int mods)
        {
            if (action == InputState::Press) {
                mousePressed[button] = true;
            } else {
                mousePressed[button] = false;
            }
            if (onMouseButton) onMouseButton(button, action, mods);
        }
        void _scrollCallback(const double xoffset, const double yoffset)
        {
            scroll = yoffset;
            if (onScroll) onScroll(xoffset, yoffset);
        }

        double xpos = 0.0;
        double ypos = 0.0;
        double xlast = 0.0;
        double ylast = 0.0;
        double xoffset = 0.0;
        double yoffset = 0.0;
        double scroll = 0.0;
        bool mousePressed[2] = { false, false };

    private:
        std::function<void(const int, const int, const int, const int)> onKey;
        std::function<void(const double, const double)> onCursorPosition;
        std::function<void(const int, const int, const int)> onMouseButton;
        std::function<void(const double, const double)> onScroll;

        void setInputCallbacks();

        GLFWwindow* window = nullptr;
    };
}
