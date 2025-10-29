#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> // Para lookAt

class Camera {
public:
    Camera() = default;
    
    void handleMouseMovement(double xpos, double ypos, bool mousePressed) noexcept;
    void handleScroll(double yoffset) noexcept;
    void update() noexcept { /* ... */ }
    
    [[nodiscard]] glm::mat4 getViewMatrix() const noexcept;
    [[nodiscard]] float getRadius() const noexcept { return radius; }
    [[nodiscard]] glm::vec3 getPosition() const noexcept;
    
private:
    float yaw = 90.0f;
    float pitch = 0.0f;
    float lastX = 400.0f;
    float lastY = 300.0f;
    bool firstMouse = true;
    float radius = 8.0f;
    
    [[nodiscard]] glm::vec3 calculateCameraPosition() const noexcept;
};