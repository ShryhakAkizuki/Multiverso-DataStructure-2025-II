#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <numbers>

class Geometry {
public:
    // Esferas
    [[nodiscard]] static std::vector<float> generateSphereVertices(
        float radius = 0.2f, 
        unsigned int stacks = 16, 
        unsigned int sectors = 32);
    
    [[nodiscard]] static std::vector<unsigned int> generateSphereIndices(
        unsigned int stacks = 16, 
        unsigned int sectors = 32);
    
    // Flechas
    [[nodiscard]] static std::vector<float> generateArrowVertices(
        float shaftLength = 0.8f,
        float shaftRadius = 0.02f,
        float headLength = 0.2f,
        float headRadius = 0.05f,
        unsigned int segments = 16);
    
    [[nodiscard]] static std::vector<unsigned int> generateArrowIndices(
        unsigned int segments = 16);
};