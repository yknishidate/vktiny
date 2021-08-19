#include "vktiny/Input.hpp"
#include "vktiny/Log.hpp"

namespace
{
    void keyCallback(GLFWwindow* window, const int key, const int scancode, const int action, const int mods)
    {
        auto* const this_ = static_cast<vkt::Input*>(glfwGetWindowUserPointer(window));
        this_->_keyCallback(key, scancode, action, mods);
    }

    void cursorPositionCallback(GLFWwindow* window, const double xpos, const double ypos)
    {
        auto* const this_ = static_cast<vkt::Input*>(glfwGetWindowUserPointer(window));
        this_->_cursorPositionCallback(xpos, ypos);
    }

    void mouseButtonCallback(GLFWwindow* window, const int button, const int action, const int mods)
    {
        auto* const this_ = static_cast<vkt::Input*>(glfwGetWindowUserPointer(window));
        this_->_mouseButtonCallback(button, action, mods);
    }

    void scrollCallback(GLFWwindow* window, const double xoffset, const double yoffset)
    {
        auto* const this_ = static_cast<vkt::Input*>(glfwGetWindowUserPointer(window));
        this_->_scrollCallback(xoffset, yoffset);
    }
}

void vkt::Input::setInputCallbacks()
{
    glfwSetWindowUserPointer(window, this);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetCursorPosCallback(window, cursorPositionCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetScrollCallback(window, scrollCallback);
}
