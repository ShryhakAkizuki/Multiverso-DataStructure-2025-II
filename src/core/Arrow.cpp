#include "core/Arrow.hpp"
#include "core/Node.hpp" 

#include <glm/gtc/matrix_transform.hpp> // Para glm::translate(), glm::rotate(), glm::scale() y glm::half_pi()

void Arrow::updateTransform(float sphereRadius, float thickness){
        if (!_origen || !_destino) {
            _transform = glm::mat4(1.0f);
            return;
        }
        
        const glm::vec3& startPos = _origen->_posicion;
        const glm::vec3& endPos = _destino->_posicion;
        
        const glm::vec3 direction = endPos - startPos;
        const float distance = glm::length(direction);
        
        if (distance < 1e-5f) {
            _transform = glm::mat4(1.0f);
            return;
        }
        
        const glm::vec3 dirNormalized = direction / distance;
        
        // Ajustar por radios de las esferas
        const glm::vec3 start = startPos + dirNormalized * sphereRadius;
        const glm::vec3 end = endPos - dirNormalized * sphereRadius;
        
        const glm::vec3 adjustedDir = end - start;
        const float adjustedLen = glm::length(adjustedDir);
        
        if (adjustedLen < 1e-5f) {
            _transform = glm::mat4(1.0f);
            return;
        }
        
        const glm::vec3 adjustedDirNorm = adjustedDir / adjustedLen;
        
        // Matriz de transformación
        const glm::mat4 translation = glm::translate(glm::mat4(1.0f), start);
        
        // Rotación para alinear el eje Y con la dirección
        const glm::vec3 up(0.0f, 1.0f, 0.0f);
        const glm::vec3 axis = glm::cross(up, adjustedDirNorm);
        const float dotProduct = glm::dot(up, adjustedDirNorm);
        const float rotationAngle = std::acos(std::clamp(dotProduct, -1.0f, 1.0f));
        
        glm::mat4 rotation = glm::mat4(1.0f);
        if (glm::length(axis) > 1e-6f) {
            rotation = glm::rotate(glm::mat4(1.0f), rotationAngle, glm::normalize(axis));
        }
        
        const glm::mat4 scale = glm::scale(glm::mat4(1.0f), 
                                          glm::vec3(thickness, adjustedLen, thickness));
        
        // Rotación base para que la flecha apunte correctamente
        const glm::mat4 baseRotation = glm::rotate(glm::mat4(1.0f), 
                                                  -glm::half_pi<float>(), 
                                                  glm::vec3(1.0f, 0.0f, 0.0f));
        
        _transform = translation * rotation * scale * baseRotation;
    }