#pragma once
#include <numbers>
#include <algorithm> 

#include <glm/glm.hpp>                  // Para glm::vec3, glm::mat4

class Node; // Forward declaration

class Arrow {
public:
    // ----- Atributos -----    
    const Node* _origen = nullptr;
    const Node* _destino = nullptr;
    glm::mat4 _transform = glm::mat4(1.0f);
    glm::vec3 _color = glm::vec3(1.0f);
    
    // ----- Constructores -----
    Arrow() = default;                                                                              // Constructor por defecto
    Arrow(const Arrow& other) = default;                                                            // Constructor de copia
    Arrow(Arrow&& other) noexcept = default;                                                        // Constructor de movimiento
    
    Arrow(const Node* origen, const Node* destino, const glm::vec3& color = glm::vec3(1.0f)) :      // Constructor por inicializacion
    _origen(origen), _destino(destino), _color(color) {
        updateTransform();
    }
    
    // ----- Destructor -----
    ~Arrow() = default;

    // ----- Operadores -----
    Arrow& operator=(const Arrow& other) = default;                                                 // Asignacion por copia
    Arrow& operator=(Arrow&& other) noexcept = default;                                             // Asignacion por movimiento

    // ----- Metodos -----
    void updateTransform(float sphereRadius = 0.2f, float thickness = 1.0f);
};