#include "Camera.hpp"
#include <glm/gtc/matrix_transform.hpp>

void Camera::handleMouseMovement(double xpos, double ypos, bool mousePressed) noexcept {
    if (!mousePressed) {
        firstMouse = true;
        return;
    }
    
    if (firstMouse) {
        lastX = static_cast<float>(xpos);
        lastY = static_cast<float>(ypos);
        firstMouse = false;
    }
    
    float xoffset = static_cast<float>(xpos) - lastX;
    float yoffset = lastY - static_cast<float>(ypos);
    lastX = static_cast<float>(xpos);
    lastY = static_cast<float>(ypos);
    
    const float sensitivity = 0.2f;
    yaw += xoffset * sensitivity;
    pitch += yoffset * sensitivity;
    
    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;
}

void Camera::handleScroll(double yoffset) noexcept {
    radius -= static_cast<float>(yoffset) * 0.5f;
    if (radius < 2.0f) radius = 2.0f;
    if (radius > 50.0f) radius = 50.0f;
}

glm::vec3 Camera::calculateCameraPosition() const noexcept {
    float camX = radius * cosf(glm::radians(yaw)) * cosf(glm::radians(pitch));
    float camY = radius * sinf(glm::radians(pitch));
    float camZ = radius * sinf(glm::radians(yaw)) * cosf(glm::radians(pitch));
    return glm::vec3(camX, camY, camZ);
}

glm::mat4 Camera::getViewMatrix() const noexcept {
    glm::vec3 cameraPos = calculateCameraPosition();
    return glm::lookAt(cameraPos, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
}

glm::vec3 Camera::getPosition() const noexcept {
    return calculateCameraPosition();
}