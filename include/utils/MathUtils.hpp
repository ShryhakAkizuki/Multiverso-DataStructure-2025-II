#pragma once
#include <glm/glm.hpp>
#include <random>
#include <concepts>

namespace MathUtils {
    // Conceptos C++20
    template<typename T>
    concept Arithmetic = std::is_arithmetic_v<T>;
    
    // Generación de colores por nivel (HSV to RGB)
    [[nodiscard]] static glm::vec3 levelToColor(float level, float maxLevel, 
                                               float saturation = 0.8f, 
                                               float value = 1.0f) {
        float hue = level / (maxLevel + 1.0f);
        float c = value * saturation;
        float m = value - c;
        float x = c * (1.0f - std::abs(std::fmod(hue * 6.0f, 2.0f) - 1.0f));
        
        if (hue < 1.0f / 6.0f) return {c + m, x + m, m};
        else if (hue < 2.0f / 6.0f) return {x + m, c + m, m};
        else if (hue < 3.0f / 6.0f) return {m, c + m, x + m};
        else if (hue < 4.0f / 6.0f) return {m, x + m, c + m};
        else if (hue < 5.0f / 6.0f) return {x + m, m, c + m};
        else return {c + m, m, x + m};
    }
    
    // Distribución uniforme con concept
    template<Arithmetic T>
    [[nodiscard]] static T randomRange(T min, T max) {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        
        if constexpr (std::is_integral_v<T>) {
            std::uniform_int_distribution<T> dist(min, max);
            return dist(gen);
        } else {
            std::uniform_real_distribution<T> dist(min, max);
            return dist(gen);
        }
    }
    
    // Fibonacci sphere distribution para posiciones 3D
    [[nodiscard]] static std::vector<glm::vec3> distributeOnSphere(int count, float radius) {
        std::vector<glm::vec3> points;
        points.reserve(count);
        
        const float goldenAngle = std::numbers::pi_v<float> * (3.0f - std::sqrt(5.0f));
        
        for (int i = 0; i < count; ++i) {
            float y = 1.0f - (i / float(count - 1)) * 2.0f;
            float radiusAtY = std::sqrt(1.0f - y * y);
            
            float theta = goldenAngle * i;
            float x = std::cos(theta) * radiusAtY;
            float z = std::sin(theta) * radiusAtY;
            
            points.emplace_back(x * radius, y * radius, z * radius);
        }
        
        return points;
    }
}