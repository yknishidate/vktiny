#pragma once
#include <GLFW/glfw3.h>

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

        void initialize(GLFWwindow* window)
        {
            this->window = window;
        }

        void setOnKey(std::function<void(const int, const int, const int, const int)> onKey)
        {
            this->onKey = onKey;
        }

        void setOnCursorPosition(std::function<void(const double, const double)> onCursorPosition)
        {
            this->onCursorPosition = onCursorPosition;
        }

    private:
        std::function<void(const int, const int, const int, const int)> onKey;
        std::function<void(const double, const double)> onCursorPosition;
        //void onKey(const int key, const int scancode, const int action, const int mods);
        //void onCursorPosition(const double xpos, const double ypos);
        //void onMouseButton(const int button, const int action, const int mods);
        //void onScroll(const double xoffset, const double yoffset);

        void setInputCallbacks();

        GLFWwindow* window;
    };
}
