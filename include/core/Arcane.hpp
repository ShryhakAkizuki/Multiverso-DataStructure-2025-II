#pragma once
#include "Node.hpp"
#include "Arrow.hpp"
#include "DinamicArray.hpp"

#include <random>
#include <cstdint>
#include <memory>

#include <glm/glm.hpp>           // Para glm::vec3, glm::mat4
#include <glm/gtc/constants.hpp> // Para constantes matemáticas

class Arcane {
private:
    // ----- Atributos -----
    DinamicArray<Node> _nodos;
    DinamicArray<Arrow> _flechas;
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
    DinamicArray<const Node*> findPath(uint32_t idOrigen, uint32_t idDestino) const;
    DinamicArray<glm::vec3> highlightPath(const DinamicArray<const Node*>& path, 
                                            glm::vec3 highlightColor = glm::vec3(1.0f));

    // Datos para renderizado
    DinamicArray<glm::vec3> getNodePositions() const;
    DinamicArray<glm::vec3> getNodeColors() const;
    DinamicArray<glm::mat4> getArrowTransforms() const;
    DinamicArray<glm::vec3> getArrowColors() const;
    
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