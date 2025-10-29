#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <memory>
#include <array>

class Arcane;
class ShaderManager;

class Renderer {
public:
    Renderer();
    ~Renderer();
    
    bool initialize(GLFWwindow* window);
    void render(const Arcane& arcane);
    void cleanup();
    
    void setViewMatrix(const glm::mat4& view) noexcept { this->view = view; }
    void setProjectionMatrix(const glm::mat4& projection) noexcept { this->projection = projection; }
    
    [[nodiscard]] static glm::mat4 calculateProjection(float aspectRatio) noexcept;
    
private:
    GLFWwindow* window = nullptr;
    glm::mat4 view;
    glm::mat4 projection;
    std::unique_ptr<ShaderManager> shaderManager;
    
    struct MeshBuffers {
        GLuint VAO = 0, VBO = 0, EBO = 0;
        std::array<GLuint, 2> instanceVBOs{};
        
        void cleanup() noexcept;
    };
    
    MeshBuffers sphereBuffers;
    MeshBuffers arrowBuffers;
    
    void setupSphereBuffers();
    void setupArrowBuffers();
    void updateSphereInstances(const Arcane& arcane);
    void updateArrowInstances(const Arcane& arcane);
    
    // Shaders como strings normales
    static constexpr const char* ARROW_VERTEX_SHADER = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        layout (location = 3) in vec3 instanceColor;
        layout (location = 4) in mat4 instanceMatrix;

        out vec3 fragColor;

        uniform mat4 view;
        uniform mat4 projection;

        void main() {
            gl_Position = projection * view * instanceMatrix * vec4(aPos, 1.0);
            fragColor = instanceColor;
        }
    )";
    
    static constexpr const char* ARROW_FRAGMENT_SHADER = R"(
        #version 330 core
        in vec3 fragColor;
        out vec4 FragColor;
        void main() {
            FragColor = vec4(fragColor, 1.0);
        }
    )";

    // Añadir shaders para esferas
    static constexpr const char* SPHERE_VERTEX_SHADER = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        layout (location = 2) in vec3 instancePos;
        layout (location = 3) in vec3 instanceColor;

        uniform mat4 mvp;
        out vec3 vColor;

        void main() {
            vec4 worldPos = vec4(aPos + instancePos, 1.0);
            gl_Position = mvp * worldPos;
            vColor = instanceColor;
        }
    )";

    static constexpr const char* SPHERE_FRAGMENT_SHADER = R"(
        #version 330 core
        in vec3 vColor;
        out vec4 FragColor;
        void main() {
            FragColor = vec4(vColor, 1.0);
        }
    )";
};