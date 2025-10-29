#pragma once
#include "DinamicArray.hpp"

#include <cstdint>

#include <glm/glm.hpp>                  // Para glm::vec3, glm::mat4

class Node {
public:
    // ----- Atributos -----    
    uint32_t _id = -1;
    uint32_t _level = -1;
    uint8_t  _conexiones = 0;

    glm::vec3 _posicion = glm::vec3(0.0f);
    glm::vec3 _color = glm::vec3(1.0f);
    
    DinamicArray<Node*> _output;
    DinamicArray<Node*> _input;
    
    // ----- Constructores -----
    Node() = default;                                                               // Constructor por defecto
    Node(Node&& other) noexcept = default;                                          // Constructor de movimiento

    Node(uint32_t id, uint32_t level, uint8_t num_outputs, uint8_t num_inputs) :    // Constructor de inicializacion
        _id(id), _level(level),
        _conexiones(num_outputs + num_inputs) {
        
        _output = DinamicArray<Node*>(num_outputs);
        _input = DinamicArray<Node*>(num_inputs);
    }

    Node(const Node& other) :                                                       // Constructor de copia
        _id(other._id), _level(other._level),
        _conexiones(other._conexiones),
        _posicion(other._posicion), _color(other._color),
        _output(other._output), _input(other._input) {}  
    
    // ----- Destructores -----
    ~Node() = default;

    // ----- Operadores -----
    Node& operator=(const Node& other) {                                            // Asignacion por copia
        if (this != &other) {
            _id = other._id;
            _level = other._level;
            _conexiones = other._conexiones;
            _posicion = other._posicion;
            _color = other._color;
            _output = other._output;  // DinamicArray maneja la copia
            _input = other._input;    // DinamicArray maneja la copia
        }
        return *this;
    }
    
    Node& operator=(Node&& other) noexcept = default;                               // Asignacion por movimiento
};