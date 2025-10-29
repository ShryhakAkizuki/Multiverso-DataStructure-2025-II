#pragma once
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <functional>
#include <unordered_map>

class InputHandler {
public:
    using MouseCallback = std::function<void(double, double)>;
    using ScrollCallback = std::function<void(double)>;
    using KeyCallback = std::function<void(int, int)>;
    
    InputHandler() = default;
    
    void setupCallbacks(GLFWwindow* window);
    void update();
    
    // Registro de callbacks
    void registerMouseCallback(MouseCallback callback) { mouseCallbacks.push_back(callback); }
    void registerScrollCallback(ScrollCallback callback) { scrollCallbacks.push_back(callback); }
    void registerKeyCallback(int key, KeyCallback callback) { keyCallbacks[key] = callback; }
    
    // Estado actual del input
    [[nodiscard]] bool isKeyPressed(int key) const noexcept { return keysPressed[key]; }
    [[nodiscard]] glm::dvec2 getMousePosition() const noexcept { return mousePos; }
    [[nodiscard]] bool isMousePressed(int button) const noexcept { return mouseButtons[button]; }
    
private:
    GLFWwindow* window = nullptr;
    
    // Estado del input
    std::unordered_map<int, bool> keysPressed;
    std::unordered_map<int, bool> mouseButtons;
    glm::dvec2 mousePos{0.0, 0.0};
    glm::dvec2 mouseDelta{0.0, 0.0};
    
    // Callbacks
    std::vector<MouseCallback> mouseCallbacks;
    std::vector<ScrollCallback> scrollCallbacks;
    std::unordered_map<int, KeyCallback> keyCallbacks;
    
    // GLFW callbacks estáticos
    static void staticKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void staticMouseCallback(GLFWwindow* window, double xpos, double ypos);
    static void staticScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
    static void staticMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    
    // Instancia para callbacks estáticos
    static InputHandler* getInstance(GLFWwindow* window);
};