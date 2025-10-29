#include "Geometry.hpp"
#include <numbers>
#include <algorithm>

std::vector<float> Geometry::generateSphereVertices(float radius, unsigned int stacks, unsigned int sectors) {
    std::vector<float> vertices;
    vertices.reserve((stacks + 1) * (sectors + 1) * 3);
    
    for (unsigned int i = 0; i <= stacks; ++i) {
        float stackAngle = std::numbers::pi_v<float> / 2.0f - i * std::numbers::pi_v<float> / stacks;
        float xy = radius * std::cos(stackAngle);
        float z = radius * std::sin(stackAngle);
        
        for (unsigned int j = 0; j <= sectors; ++j) {
            float sectorAngle = j * 2.0f * std::numbers::pi_v<float> / sectors;
            float x = xy * std::cos(sectorAngle);
            float y = xy * std::sin(sectorAngle);
            vertices.insert(vertices.end(), {x, y, z});
        }
    }
    return vertices;
}

std::vector<unsigned int> Geometry::generateSphereIndices(unsigned int stacks, unsigned int sectors) {
    std::vector<unsigned int> indices;
    indices.reserve(stacks * sectors * 6);
    
    for (unsigned int i = 0; i < stacks; ++i) {
        unsigned int k1 = i * (sectors + 1);
        unsigned int k2 = k1 + sectors + 1;
        
        for (unsigned int j = 0; j < sectors; ++j, ++k1, ++k2) {
            if (i != 0) {
                indices.insert(indices.end(), {k1, k2, k1 + 1});
            }
            if (i != stacks - 1) {
                indices.insert(indices.end(), {k1 + 1, k2, k2 + 1});
            }
        }
    }
    return indices;
}

std::vector<float> Geometry::generateArrowVertices(float shaftLength, float shaftRadius, 
                                                  float headLength, float headRadius, 
                                                  unsigned int segments) {
    std::vector<float> vertices;
    vertices.reserve((segments + 1) * 2 * 3 + (segments + 1) * 3 + 3);
    
    const float PI = std::numbers::pi_v<float>;
    
    // Cilindro (shaft)
    for (unsigned int i = 0; i <= segments; ++i) {
        float theta = 2.0f * PI * i / segments;
        float x = std::cos(theta);
        float y = std::sin(theta);
        vertices.insert(vertices.end(), {x * shaftRadius, y * shaftRadius, 0.0f});
        vertices.insert(vertices.end(), {x * shaftRadius, y * shaftRadius, shaftLength});
    }
    
    // Base del cono (head)
    for (unsigned int i = 0; i <= segments; ++i) {
        float theta = 2.0f * PI * i / segments;
        float x = std::cos(theta);
        float y = std::sin(theta);
        vertices.insert(vertices.end(), {x * headRadius, y * headRadius, shaftLength});
    }
    
    // Ápice del cono
    vertices.insert(vertices.end(), {0.0f, 0.0f, shaftLength + headLength});
    
    return vertices;
}

std::vector<unsigned int> Geometry::generateArrowIndices(unsigned int segments) {
    std::vector<unsigned int> indices;
    indices.reserve(segments * 6 + segments * 3);
    
    // Cilindro
    for (unsigned int i = 0; i < segments; ++i) {
        unsigned int base = i * 2;
        indices.insert(indices.end(), {
            base, base + 1, base + 3,
            base, base + 3, base + 2
        });
    }
    
    // Cono
    unsigned int offset = (segments + 1) * 2;
    for (unsigned int i = 0; i < segments; ++i) {
        unsigned int a = offset + i;
        unsigned int b = offset + (i + 1) % (segments + 1);
        unsigned int apex = offset + (segments + 1);
        indices.insert(indices.end(), {a, b, apex});
    }
    
    return indices;
}