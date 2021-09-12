#include "vktiny/Scene/Camera.hpp"

namespace vkt
{
    OrbitalCamera::OrbitalCamera(int width, int height, float distance,
                                 glm::vec3 target)
    {
        this->distance = distance;
        target = target;
        aspect = float(width) / height;
        update();
    }

    void OrbitalCamera::update()
    {
        if (distance < 0.01) {
            view = glm::lookAt(glm::vec3(0.0), target, glm::vec3(0, 1, 0));
            proj = glm::perspective(glm::radians(fov), aspect, 0.01f, 10000.0f);
            return;
        }
        // Update
        glm::mat4 rotX = glm::rotate(glm::radians(theta), glm::vec3(1, 0, 0));
        glm::mat4 rotY = glm::rotate(glm::radians(phi), glm::vec3(0, 1, 0));

        auto pos = glm::vec3(rotY * rotX * glm::vec4(0, 0, distance, 1));
        view = glm::lookAt(pos, target, glm::vec3(0, 1, 0));
        proj = glm::perspective(glm::radians(fov), aspect, 0.01f, 10000.0f);
    }

    void OrbitalCamera::processCursorMotion(double xMotion, double yMotion)
    {
        phi = glm::mod(phi - float(xMotion), 360.0f);
        theta = std::min(std::max(theta + float(yMotion), -89.9f), 89.9f);
    }

    void OrbitalCamera::processMouseWheel(float value)
    {
        if (value == 0) return;
        if (value < 0) {
            distance = distance * 1.1f;
        } else {
            distance = std::max(distance * 0.9f, 0.001f);
        }
    }

    // FPSCamera
    FPSCamera::FPSCamera(int width, int height)
    {
        front = glm::vec3(0);
        up = glm::vec3(0, 1, 0);
        aspect = float(width) / height;
        proj = glm::perspective(glm::radians(fov), aspect, 0.01f, 10000.0f);

        position = glm::vec4(0, -1, 0, 1);
        yaw = 100.0f;
        pitch = 2.0f;

        update();
    }

    void FPSCamera::update()
    {
        glm::mat4 rotationMatrix(1.0);
        rotationMatrix *= glm::rotate(glm::radians(yaw), glm::vec3(0, 1, 0));
        rotationMatrix *= glm::rotate(glm::radians(pitch), glm::vec3(1, 0, 0));
        front = glm::vec3(rotationMatrix * glm::vec4(0, 0, -1, 1));

        view = glm::lookAt(glm::vec3(position), glm::vec3(position) + front, up);
    }

    void FPSCamera::processCursorMotion(double xMotion, double yMotion)
    {
        yaw = glm::mod(yaw - float(xMotion) * rotSpeed, 360.0f);
        pitch = std::min(std::max(pitch + float(yMotion) * rotSpeed, -89.9f), 89.9f);
        //update();
    }

    void FPSCamera::processMouseWheel(float value)
    {
    }

    void FPSCamera::processKeyState()
    {
        glm::vec3 forward = glm::normalize(glm::vec3(front.x, 0, front.z));
        glm::vec3 right = glm::normalize(glm::cross(-up, forward));

        //if (InputSystem::getKey(GLFW_KEY_W) == GLFW_PRESS) {
        //    position += glm::vec4(forward, 0.0) * moveSpeed;
        //}
        //if (InputSystem::getKey(GLFW_KEY_S) == GLFW_PRESS) {
        //    position -= glm::vec4(forward, 0.0) * moveSpeed;
        //}
        //if (InputSystem::getKey(GLFW_KEY_D) == GLFW_PRESS) {
        //    position += glm::vec4(right, 0.0) * moveSpeed * 0.5f;
        //}
        //if (InputSystem::getKey(GLFW_KEY_A) == GLFW_PRESS) {
        //    position -= glm::vec4(right, 0.0) * moveSpeed * 0.5f;
        //}
        update();
    }
}
