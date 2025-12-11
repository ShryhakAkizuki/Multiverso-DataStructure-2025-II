#include "core/Arcane.hpp"
#include "graphics/Renderer.hpp"
#include "ui/GUI.hpp"
#include "utils/Camera.hpp"
#include <GLFW/glfw3.h>
#include <iostream>
#include <memory>

// Variables globales para input
double mouseX = 0.0, mouseY = 0.0;
double scrollY = 0.0;
bool mousePressed = false;

// Callbacks globales
void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    mouseX = xpos;
    mouseY = ypos;
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    scrollY = yoffset;
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        mousePressed = (action == GLFW_PRESS);
    }
}

int main() {
    // Inicialización GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 8);
    
    auto* window = glfwCreateWindow(1920, 1080, "Arcane 3D Viewer", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // VSync
    
    // Configurar callbacks
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    
    // Inicializar GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        glfwTerminate();
        return -1;
    }
    
    std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;
    
    // Inicializar componentes
    Renderer renderer;
    GUI gui;
    Camera camera;
    
    if (!renderer.initialize(window)) {
        std::cerr << "Failed to initialize renderer\n";
        glfwTerminate();
        return -1;
    }
    
    gui.initialize(window);
    
    // Crear modelo
    Arcane arcane(36, 2);
    
    std::cout << "Arcane initialized with " << arcane.getNumNodes() << " nodes" << std::endl;
    
    // Loop principal
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        
        // Input handling
        camera.handleMouseMovement(mouseX, mouseY, mousePressed);
        camera.handleScroll(scrollY);
        scrollY = 0.0; // Reset scroll
        
        // Configurar matrices de vista y proyección
        renderer.setViewMatrix(camera.getViewMatrix());
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        renderer.setProjectionMatrix(Renderer::calculateProjection(static_cast<float>(width)/height));
        
        // UI updates
        gui.beginFrame();
        gui.render(arcane);
        
        // Regenerar red si se solicita
        if (auto params = gui.getRegenerationParams()) {
            auto [nodeCount, initialNodes] = *params;
            std::cout << "Reconstruyendo con " << nodeCount << " nodos, " << initialNodes << " y nodos iniciales" << std::endl;
            arcane = Arcane(nodeCount, initialNodes);
        }
        
        //Buscar ruta si se solicita
        if (gui.isPathFindingRequested()) {
            auto [node1, node2] = gui.getSelectedNodes();
            std::cout << "Buscando camino entre " << node1 << " y " << node2 << std::endl;
            auto path = arcane.findPath(node1, node2);
            if (!path.empty()) {
                std::cout << "Camino encontrado con " << path.size() << " 5 nodos" << std::endl;
            } else {
                std::cout << "No se encontro camino" << std::endl;
            }
            arcane.highlightPath(path);
        }
        
        // Render
        renderer.render(arcane);
        
        gui.endFrame();
        glfwSwapBuffers(window);
    }
    
    // Cleanup
    renderer.cleanup();
    gui.cleanup();
    glfwTerminate();
    
    return 0;
}