#pragma once
#include "vktiny/Math.hpp"

namespace vkt
{
    class Camera
    {
    public:
        Camera() = default;
        Camera(const Camera&) = delete;
        Camera(Camera&&) = default;
        Camera& operator = (const Camera&) = delete;
        Camera& operator = (Camera&&) = default;

        virtual ~Camera() = default;

        virtual void update() = 0;
        virtual void processCursorMotion(double xMotion, double yMotion) {}
        virtual void processMouseWheel(float value) {}
        virtual void processKeyState() {}

        glm::mat4 view;
        glm::mat4 proj;

        float fov = 45;
        float aspect = 1;
    };

    class OrbitalCamera : public Camera
    {
    public:
        OrbitalCamera() = default;
        OrbitalCamera(int width, int height, float distance = 10, glm::vec3 target = glm::vec3(0.0));

        void update() override;
        void processCursorMotion(double xMotion, double yMotion) override;
        void processMouseWheel(float value) override;

        glm::vec3 target;
        float distance = 10;
        float phi = 0;
        float theta = 0;
    };

    class FPSCamera : public Camera
    {
    public:
        FPSCamera() = default;
        FPSCamera(int width, int height);

        void update() override;
        void processCursorMotion(double xMotion, double yMotion) override;
        void processMouseWheel(float value) override;
        void processKeyState() override;

        glm::vec4 position;
        glm::vec3 front;
        glm::vec3 up;
        float rotSpeed = 0.1;
        float moveSpeed = 0.2;
        float pitch = 0;
        float yaw = 0;
    };
}
