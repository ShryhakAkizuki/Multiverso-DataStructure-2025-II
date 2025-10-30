#include "core/Arcane.hpp"
#include "utils/MathUtils.hpp"

#include <iostream>
#include <algorithm>



#include <glm/gtc/matrix_transform.hpp> // Para glm::translate(), glm::rotate(), glm::scale() y glm::half_pi()


// #include <queue>
// #include <ranges>
// #include <unordered_map>


Arcane::Arcane(){
    std::random_device rd;
    _gen.seed(rd());
    
    _nodos.reserve(36);
    initializeNodes();
    connectNodes();
    assign3DPositions();
    assignLevelColors();
    generateArrows();
    assignArrowColors();
}

Arcane::Arcane(uint32_t numNodosParam, uint32_t nodosIniciales) {
    
    std::random_device rd;
    _gen.seed(rd());
    
    _nodos.reserve(std::max(36u, numNodosParam)); 
    initializeNodes(nodosIniciales);
    connectNodes();
    assign3DPositions();
    assignLevelColors();
    generateArrows();
    assignArrowColors();
}

// Arcane::Arcane(const Arcane& other)
// :_niveles(other._niveles), _gen(other._gen){
// }

Arcane::Arcane(Arcane&& other) noexcept
: _nodos(std::move(other._nodos)), _flechas(std::move(other._flechas)),      
_niveles(other._niveles), _gen(std::move(other._gen)) {
    other._niveles = 0;
}

Arcane& Arcane::operator=(Arcane&& other) noexcept {
    if (this != &other) {
        _nodos = std::move(other._nodos);
        _flechas = std::move(other._flechas);
        _niveles = other._niveles;
        _gen = std::move(other._gen);
        other._niveles = 0;
    }
    return *this;
}

DynamicArray<glm::vec3> Arcane::getNodePositions() const {
    DynamicArray<glm::vec3> positions;
    positions.reserve(_nodos.size());
    for (uint32_t i = 0; i < _nodos.size(); ++i) {
        positions.push_back(_nodos[i]._posicion);
    }
    return positions;
}

DynamicArray<glm::vec3> Arcane::getNodeColors() const {
    DynamicArray<glm::vec3> colors;
    colors.reserve(_nodos.size());
    for (const auto& nodo : _nodos) {
        colors.push_back(nodo._color);
    }
    return colors;
}

DynamicArray<glm::mat4> Arcane::getArrowTransforms() const {
    DynamicArray<glm::mat4> transforms;
    transforms.reserve(_flechas.size());
    for (const auto& arrow : _flechas) {
        transforms.push_back(arrow._transform);
    }
    return transforms;
}

DynamicArray<glm::vec3> Arcane::getArrowColors() const {
    DynamicArray<glm::vec3> colors;
    colors.reserve(_flechas.size());
    for (const auto& arrow : _flechas) {
        colors.push_back(arrow._color);
    }
    return colors;
}

void Arcane::initializeNodes(uint32_t nodosIniciales) {
    if (nodosIniciales < 2) nodosIniciales = 2;
    
    uint32_t limite = nodosIniciales, acumulado = limite;
    std::uniform_int_distribution<uint32_t> distConn(2, 6);
    
    _niveles = 0;
    _nodos.clear();

    // ----- ASIGNAR NIVELES A LOS NODOS -----
    for (uint32_t i = 0; i < _nodos.capacity(); ++i) {
        uint32_t nivel = _niveles;
        
        if (i >= acumulado - 1 && i < _nodos.capacity() - 1) {
            ++_niveles;
            limite *= 2;
            acumulado += limite;
        }

        uint32_t conexiones = distConn(_gen);
        uint32_t maxIn = std::max<uint32_t>(1, conexiones - 1);
        std::uniform_int_distribution<uint32_t> distInputs(1, maxIn);
        uint32_t inputs = distInputs(_gen);
        uint32_t outputs = conexiones - inputs;

        // Crear nodo directamente con todos los parámetros
        _nodos.push_back(Node(i, nivel, outputs, inputs));
    }

}

bool Arcane::safeConnection(Node* origen, Node* destino) {
    if (!origen || !destino || origen == destino) return false;
    
    // ----- VERIFICAR SI YA EXISTE LA CONEXIÓN -----
    
    // Verificar si origen ya apunta a destino
    for (uint32_t i = 0; i < origen->_output.size(); ++i) {
        if (origen->_output[i] == destino) return false;
    }
    
    // Verificar si destino ya apunta a origen
    for (uint32_t i = 0; i < destino->_output.size(); ++i) {
        if (destino->_output[i] == origen) return false;
    }
    
    // ----- BUSCAR ESPACIOS DISPONIBLES -----
    
    // Si no hay capacidad disponible en ambos arrays
    if (origen->_output.full() || destino->_input.full()) {
        return false;
    }
    
    origen->_output.push_back(destino);
    destino->_input.push_back(origen);

    return true;
}

bool Arcane::forcedConnection(Node* origen, Node* destino) {
    if (!origen || !destino || origen == destino) return false;
    
    const uint8_t MAX_CONN = 6;
    
    // ----- EXPANDIR CAPACIDAD DE ORIGEN SI ES NECESARIO Y POSIBLE -----
    if (origen->_conexiones < MAX_CONN && origen->_output.full()) {
        uint32_t nuevaCapacidad = origen->_output.capacity() + 1;
        origen->_output.reserve(nuevaCapacidad);
        origen->_conexiones = static_cast<uint8_t>(nuevaCapacidad);  // ✅ Solo aquí se actualiza
    }
    
    // ----- EXPANDIR CAPACIDAD DE DESTINO SI ES NECESARIO Y POSIBLE -----
    if (destino->_conexiones < MAX_CONN && destino->_input.full()) {
        uint32_t nuevaCapacidad = destino->_input.capacity() + 1;
        destino->_input.reserve(nuevaCapacidad);
        destino->_conexiones = static_cast<uint8_t>(nuevaCapacidad);  // ✅ Solo aquí se actualiza
    }
    // ----- INTENTAR LA CONEXIÓN -----
    return safeConnection(origen, destino);
}

void Arcane::connectNodes() {
    const uint32_t MAX_ATTEMPTS = 8;
    const uint8_t MAX_CONN = 6;

    DynamicArray<DynamicArray<Node*>> nodesByLevel(_niveles + 1);
    std::uniform_real_distribution<float> prob(0.0f, 1.0f);

    // ----- ORGANIZAR NODOS POR NIVEL -----
    for (uint32_t lvl = 0; lvl <= _niveles; ++lvl) {
        nodesByLevel.push_back(DynamicArray<Node*>());
    }

    // Agrupar nodos por nivel
    for (uint32_t i = 0; i < _nodos.size(); ++i) {
        Node& node = _nodos[i];
        if (node._level <= _niveles) {
            nodesByLevel[node._level].push_back(&node);
        }
    }

    // ----- FASE 1: CONEXIONES PLANIFICADAS -----
    for (uint32_t i = 0; i < _nodos.size(); ++i) {
        Node& current = _nodos[i];
        uint32_t level = current._level;
        
        // Usar capacity() para obtener outputs deseados
        uint32_t wantedOutputs = current._output.capacity();
        uint32_t madeOutputs = 0;

        for (uint32_t c = 0; c < wantedOutputs; ++c) {
            bool connected = false;
            uint32_t attempts = 0;
            
            while (!connected && attempts < MAX_ATTEMPTS) {
                ++attempts;
                
                // Seleccionar nivel destino basado en probabilidades
                uint32_t targetLevel = level;
                float p = prob(_gen);
                
                if (level == 0) {
                    targetLevel = (p < 0.333f) ? level : level + 1;
                } else if (level == _niveles) {
                    targetLevel = level - 1;
                } else {
                    if (p < 0.25f) targetLevel = level - 1;
                    else if (p < 0.75f) targetLevel = level;
                    else targetLevel = level + 1;
                }
                
                if (targetLevel > _niveles) targetLevel = _niveles;
                
                // Obtener nodos del nivel destino
                DynamicArray<Node*>& targetNodes = nodesByLevel[targetLevel];
                if (targetNodes.empty()) continue;
                
                // Seleccionar nodo destino aleatorio
                std::uniform_int_distribution<uint32_t> pick(0, targetNodes.size() - 1);
                Node* target = targetNodes[pick(_gen)];
                if (!target || target == &current) continue;

                // Verificar si el destino ya apunta al actual
                bool targetPointsToCurrent = false;
                for (uint32_t k = 0; k < target->_output.size(); ++k) {
                    if (target->_output[k] == &current) {
                        targetPointsToCurrent = true;
                        break;
                    }
                }
                if (targetPointsToCurrent) continue;

                connected = safeConnection(&current, target);
            }

            // Si no se conectó, intentar conexión forzada
            if (!connected) {
                bool forced = false;
                for (uint32_t tries = 0; tries < 4 && !forced; ++tries) {
                    uint32_t targetLevel = level;
                    float p = prob(_gen);
                    
                    if (level == 0) {
                        targetLevel = (p < 0.333f) ? level : level + 1;
                    } else if (level == _niveles) {
                        targetLevel = level - 1;
                    } else {
                        if (p < 0.25f) targetLevel = level - 1;
                        else if (p < 0.75f) targetLevel = level;
                        else targetLevel = level + 1;
                    }
                    
                    if (targetLevel > _niveles) targetLevel = _niveles;
                    
                    DynamicArray<Node*>& targetNodes = nodesByLevel[targetLevel];
                    if (targetNodes.empty()) continue;
                    
                    std::uniform_int_distribution<uint32_t> pick(0, targetNodes.size() - 1);
                    Node* target = targetNodes[pick(_gen)];
                    if (!target || target == &current) continue;
                    
                    forced = forcedConnection(&current, target);
                }
                
                // Último recurso: cualquier nodo
                if (!forced) {
                    for (uint32_t j = 0; j < _nodos.size() && !forced; ++j) {
                        if (&_nodos[j] == &current) continue;
                        forced = forcedConnection(&current, &_nodos[j]);
                    }
                }
            }
        }
    }

    // ----- FASE 2: GARANTIZAR MÍNIMO 1 INPUT Y 1 OUTPUT -----
    for (uint32_t i = 0; i < _nodos.size(); ++i) {
        Node& node = _nodos[i];

        // Verificar si tiene outputs
        if (node._output.empty()) {
            bool connected = false;
            
            // Intentar niveles preferidos: nivel+1, mismo, nivel-1
            int preferredLevels[3] = { 
                static_cast<int>(node._level) + 1, 
                static_cast<int>(node._level), 
                static_cast<int>(node._level) - 1 
            };
            
            for (int pi = 0; pi < 3 && !connected; ++pi) {
                int lvl = preferredLevels[pi];
                if (lvl < 0 || lvl > static_cast<int>(_niveles)) continue;
                
                DynamicArray<Node*>& targetNodes = nodesByLevel[lvl];
                for (uint32_t t = 0; t < targetNodes.size() && !connected; ++t) {
                    Node* target = targetNodes[t];
                    if (target == &node) continue;
                    connected = forcedConnection(&node, target);
                }
            }
            
            // Último recurso: cualquier nodo
            for (uint32_t j = 0; j < _nodos.size() && !connected; ++j) {
                if (&_nodos[j] == &node) continue;
                connected = forcedConnection(&node, &_nodos[j]);
            }
        }

        // Verificar si tiene inputs
        if (node._input.empty()) {
            bool connected = false;
            
            // Intentar niveles preferidos: nivel-1, mismo, nivel+1
            int preferredLevels[3] = { 
                static_cast<int>(node._level) - 1, 
                static_cast<int>(node._level), 
                static_cast<int>(node._level) + 1 
            };
            
            for (int pi = 0; pi < 3 && !connected; ++pi) {
                int lvl = preferredLevels[pi];
                if (lvl < 0 || lvl > static_cast<int>(_niveles)) continue;
                
                DynamicArray<Node*>& sourceNodes = nodesByLevel[lvl];
                for (uint32_t t = 0; t < sourceNodes.size() && !connected; ++t) {
                    Node* source = sourceNodes[t];
                    if (source == &node) continue;
                    connected = forcedConnection(source, &node);
                }
            }
            
            // Último recurso: cualquier nodo
            for (uint32_t j = 0; j < _nodos.size() && !connected; ++j) {
                if (&_nodos[j] == &node) continue;
                connected = forcedConnection(&_nodos[j], &node);
            }
        }
    }

    // ----- FASE 3: REFORZAR ÚLTIMO NIVEL -----
    if (_niveles > 0) {
        uint32_t sourceLevel = _niveles - 1;
        DynamicArray<Node*>& sourceNodes = nodesByLevel[sourceLevel];
        DynamicArray<Node*>& targetNodes = nodesByLevel[_niveles];
        
        for (uint32_t j = 0; j < targetNodes.size(); ++j) {
            Node* target = targetNodes[j];
            
            if (target->_input.empty()) {
                bool connected = false;
                
                // Intentar desde penúltimo nivel
                for (uint32_t k = 0; k < sourceNodes.size() && !connected; ++k) {
                    Node* source = sourceNodes[k];
                    if (source == target) continue;
                    connected = forcedConnection(source, target);
                }
                
                // Fallback: cualquier nodo
                for (uint32_t x = 0; x < _nodos.size() && !connected; ++x) {
                    Node* source = &_nodos[x];
                    if (source == target) continue;
                    connected = forcedConnection(source, target);
                }
            }
        }
    }
}

void Arcane::assign3DPositions() {
    // Organizar nodos por nivel
    DynamicArray<DynamicArray<Node*>> nodesByLevel(_niveles + 1);
    for (uint32_t lvl = 0; lvl <= _niveles; ++lvl) {
        nodesByLevel.push_back(DynamicArray<Node*>());
    }
    
    for (Node& node : _nodos) {
        if (node._level <= _niveles) {
            nodesByLevel[node._level].push_back(&node);
        }
    }

    // Asignar posiciones por nivel
    for (uint32_t level = 0; level <= _niveles; ++level) {
        DynamicArray<Node*>& levelNodes = nodesByLevel[level];
        
        if (levelNodes.empty()) continue;
        
        float radius = static_cast<float>((level + 1) * (level + 1));
        
        for (uint32_t i = 0; i < levelNodes.size(); ++i) {
            Node* node = levelNodes[i];

            const float goldenAngle = glm::pi<float>() * (3.0f - std::sqrt(5.0f));
    
            float offset = 2.0f / static_cast<float>(levelNodes.size());
            float y = (static_cast<float>(i) * offset - 1.0f) + (offset / 2.0f);
            y = std::clamp(y, -1.0f, 1.0f);
            
            float horizontalRadius = std::sqrt(std::max(0.0f, 1.0f - y * y));
            float phi = static_cast<float>(i) * goldenAngle;
            
            float x = horizontalRadius * std::cos(phi);
            float z = horizontalRadius * std::sin(phi);
   
            node->_posicion = glm::vec3(x, y, z) * radius;      
        }
    }
}

void Arcane::assignLevelColors() {
    for (Node& node : _nodos) {
        node._color = MathUtils::levelToColor(static_cast<float>(node._level), 
                                         static_cast<float>(_niveles));
    }
}

void Arcane::generateArrows() {
    // Calcular número total de flechas para pre-reservar memoria
    uint32_t totalArrows = 0;
    for (const Node& node : _nodos) {
        totalArrows += node._output.size();
    }

    _flechas.clear();
    _flechas.reserve(totalArrows); // Pre-reservar memoria

    for (Node& originNode : _nodos) {
        for (Node* targetNode : originNode._output) {
            if (targetNode != nullptr) {
                _flechas.push_back(Arrow(&originNode, targetNode));
            }
        }
    }

    // Actualizar transformaciones de todas las flechas
    updateAllArrows();
}

void Arcane::updateAllArrows() {
    for (Arrow& arrow : _flechas) {
        arrow.updateTransform();
    }
}

void Arcane::assignArrowColors() {
    const float oscuridad = 0.7f;
    for (auto& arrow : _flechas) {
        if (arrow._origen) {
            arrow._color = arrow._origen->_color * oscuridad;
        }
    }
}

DynamicArray<const Node*> Arcane::findPath(uint32_t idOrigen, uint32_t idDestino) const {
    // Validar IDs
    if (idOrigen >= _nodos.size() || idDestino >= _nodos.size()) {
        return DynamicArray<const Node*>();
    }

    // Caso trivial
    if (idOrigen == idDestino) {
        DynamicArray<const Node*> path;
        path.push_back(&_nodos[idOrigen]);
        return path;
    }

    // Vector de padres (en lugar de std::vector<int>)
    DynamicArray<int> parent(_nodos.size());
    for (uint32_t i = 0; i < _nodos.size(); ++i) {
        parent.push_back(-1);
    }
    
    // Vector de visitados (en lugar de std::vector<char>)
    DynamicArray<bool> visited(_nodos.size());
    for (uint32_t i = 0; i < _nodos.size(); ++i) {
        visited.push_back(false);
    }
    
    // Queue manual usando DynamicArray (FIFO)
    DynamicArray<uint32_t> queue;
    queue.push_back(idOrigen);
    visited[idOrigen] = true;
    
    bool found = false;
    uint32_t queueIndex = 0;  // Índice para simular pop_front
    
    // BFS manual
    while (queueIndex < queue.size() && !found) {
        uint32_t currentId = queue[queueIndex++];
        const Node& currentNode = _nodos[currentId];
        
        // Iterar sobre conexiones de salida
        for (uint32_t k = 0; k < currentNode._output.size(); ++k) {
            const Node* neighbor = currentNode._output[k];
            if (!neighbor) continue;
            
            uint32_t neighborId = neighbor->_id;
            if (!visited[neighborId]) {
                visited[neighborId] = true;
                parent[neighborId] = static_cast<int>(currentId);
                
                if (neighborId == idDestino) {
                    found = true;
                    break;
                }
                
                queue.push_back(neighborId);
            }
        }
    }
    
    // Si no se encontró camino
    if (!found) {
        return DynamicArray<const Node*>();
    }
    
    // ----- RECONSTRUIR CAMINO -----
    DynamicArray<const Node*> path;
    int current = static_cast<int>(idDestino);
    
    // Reconstruir de destino a origen
    while (current != -1) {
        path.push_back(&_nodos[current]);
        current = parent[current];
    }
    
    // Invertir el camino (origen -> destino)
    for (uint32_t i = 0; i < path.size() / 2; ++i) {
        const Node* temp = path[i];
        path[i] = path[path.size() - 1 - i];
        path[path.size() - 1 - i] = temp;
    }
    
    return path;
}

DynamicArray<glm::vec3> Arcane::highlightPath(const DynamicArray<const Node*>& path, 
                                             glm::vec3 highlightColor) {
    DynamicArray<glm::vec3> colors;
    colors.reserve(_flechas.size());

    // Guardar colores originales temporalmente
    DynamicArray<glm::vec3> originalColors;
    originalColors.reserve(_flechas.size());
    for (const Arrow& arrow : _flechas) {
        originalColors.push_back(arrow._color);
    }

    // Reiniciar a colores base
    assignArrowColors();

    // Aplicar highlight si el path es válido
    if (path.size() >= 2) {
        for (uint32_t i = 0; i + 1 < path.size(); ++i) {
            const Node* from = path[i];
            const Node* to = path[i + 1];
            
            if (!from || !to) continue;
            
            for (Arrow& arrow : _flechas) {
                if (arrow._origen && arrow._destino &&
                    arrow._origen->_id == from->_id && 
                    arrow._destino->_id == to->_id) {
                    
                    arrow._color = highlightColor;
                    break;
                }
            }
        }
    }

    // Recoger colores resultantes
    for (const Arrow& arrow : _flechas) {
        colors.push_back(arrow._color);
    }

    return colors;
}
    