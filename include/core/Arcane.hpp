#pragma once
#include "Node.hpp"
#include "Arrow.hpp"
#include "DynamicArray.hpp"

#include <random>
#include <cstdint>
#include <memory>

#include <glm/glm.hpp>           // Para glm::vec3, glm::mat4
#include <glm/gtc/constants.hpp> // Para constantes matemáticas

class Arcane {
private:
    // ----- Atributos -----
    DynamicArray<Node> _nodos;
    DynamicArray<Arrow> _flechas;
    uint32_t _niveles = 0;
    std::mt19937 _gen;


public:
    Arcane();                                               // Constructor por defecto
    Arcane(uint32_t numNodos, uint32_t nodosIniciales);     // Constructor por inicializacion
    // Arcane(const Arcane& other);                            // Constructor de copia
    Arcane(Arcane&& other) noexcept;                        // Constructor de movimiento
    
    // ----- Destructor -----
    ~Arcane() = default;
    
    // ----- Operadores -----
    //Arcane& operator=(const Arcane&);
    Arcane& operator=(Arcane&& other) noexcept;

    // ----- Metodos -----
    
    // Retorno
    uint32_t getNumLevels() const noexcept { return _niveles + 1; }
    uint32_t getNumNodes() const noexcept { return _nodos.size(); }
    uint32_t getNumArrows() const noexcept { return _flechas.size(); }

    // Algoritmos
    DynamicArray<const Node*> findPath(uint32_t idOrigen, uint32_t idDestino) const;
    DynamicArray<glm::vec3> highlightPath(const DynamicArray<const Node*>& path, 
                                            glm::vec3 highlightColor = glm::vec3(1.0f));

    // Datos para renderizado
    DynamicArray<glm::vec3> getNodePositions() const;
    DynamicArray<glm::vec3> getNodeColors() const;
    DynamicArray<glm::mat4> getArrowTransforms() const;
    DynamicArray<glm::vec3> getArrowColors() const;
    
private:
    
    // ----- Metodos -----
    void initializeNodes(uint32_t nodosIniciales = 2);
    bool safeConnection(Node* origen, Node* destino);
    bool forcedConnection(Node* origen, Node* destino);
    void connectNodes();
    void assign3DPositions();
    void assignLevelColors();
    void generateArrows();
    void assignArrowColors();
    void updateAllArrows();
};