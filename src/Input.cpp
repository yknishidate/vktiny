#include "vktiny/Input.hpp"
#include "vktiny/Log.hpp"

namespace
{
    void keyCallback(GLFWwindow* window, const int key, const int scancode, const int action, const int mods)
    {
        auto* const this_ = static_cast<Input*>(glfwGetWindowUserPointer(window));
        this_->onKey(key, scancode, action, mods);
    }

    void cursorPositionCallback(GLFWwindow* window, const double xpos, const double ypos)
    {

    }

    void mouseButtonCallback(GLFWwindow* window, const int button, const int action, const int mods)
    {

    }

    void scrollCallback(GLFWwindow* window, const double xoffset, const double yoffset)
    {

    }
}

//void vkt::Input::onKey(const int key, const int scancode, const int action, const int mods)
//{
//}
//
//void vkt::Input::onCursorPosition(const double xpos, const double ypos)
//{
//    vkt::log::info("onCursorPosition: {} {}", xpos, ypos);
//}
//
//void vkt::Input::onMouseButton(const int button, const int action, const int mods)
//{
//}
//
//void vkt::Input::onScroll(const double xoffset, const double yoffset)
//{
//}

void vkt::Input::setInputCallbacks()
{
    glfwSetKeyCallback(window, keyCallback);
    glfwSetCursorPosCallback(window, cursorPositionCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetScrollCallback(window, scrollCallback);
}
