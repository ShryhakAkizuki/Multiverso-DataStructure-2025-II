#include "graphics/Renderer.hpp"
#include "graphics/ShaderManager.hpp"
#include "graphics/Geometry.hpp"
#include "core/Arcane.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

Renderer::Renderer() = default;

Renderer::~Renderer() {
    cleanup();
}

bool Renderer::initialize(GLFWwindow* window) {
    this->window = window;
    
    shaderManager = std::make_unique<ShaderManager>();
    
    // Cargar shaders para flechas
    auto arrowResult = shaderManager->loadShaderProgram("arrow", 
                                                       ARROW_VERTEX_SHADER, 
                                                       ARROW_FRAGMENT_SHADER);
    if (!arrowResult.success) {
        std::cerr << "Failed to load arrow shader: " << arrowResult.errorMessage << std::endl;
        return false;
    }
    
    // Cargar shaders para esferas
    auto sphereResult = shaderManager->loadShaderProgram("sphere", 
                                                        SPHERE_VERTEX_SHADER, 
                                                        SPHERE_FRAGMENT_SHADER);
    if (!sphereResult.success) {
        std::cerr << "Failed to load sphere shader: " << sphereResult.errorMessage << std::endl;
        return false;
    }
    
    setupSphereBuffers();
    setupArrowBuffers();
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glClearColor(0.05f, 0.05f, 0.1f, 1.0f);
    
    return true;
}

void Renderer::setupSphereBuffers() {
    glGenVertexArrays(1, &sphereBuffers.VAO);
    glGenBuffers(1, &sphereBuffers.VBO);
    glGenBuffers(1, &sphereBuffers.EBO);
    glGenBuffers(2, sphereBuffers.instanceVBOs.data());
    
    auto sphereVertices = Geometry::generateSphereVertices();
    auto sphereIndices = Geometry::generateSphereIndices();
    
    glBindVertexArray(sphereBuffers.VAO);
    
    // Vértices
    glBindBuffer(GL_ARRAY_BUFFER, sphereBuffers.VBO);
    glBufferData(GL_ARRAY_BUFFER, sphereVertices.size() * sizeof(float), 
                 sphereVertices.data(), GL_STATIC_DRAW);
    
    // Índices
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereBuffers.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphereIndices.size() * sizeof(unsigned int),
                 sphereIndices.data(), GL_STATIC_DRAW);
    
    // Atributos de vértice (posición)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Buffers de instancia para posiciones
    glBindBuffer(GL_ARRAY_BUFFER, sphereBuffers.instanceVBOs[0]);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(2);
    glVertexAttribDivisor(2, 1);
    
    // Buffers de instancia para colores
    glBindBuffer(GL_ARRAY_BUFFER, sphereBuffers.instanceVBOs[1]);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(3);
    glVertexAttribDivisor(3, 1);
    
    glBindVertexArray(0);
}

void Renderer::setupArrowBuffers() {
    glGenVertexArrays(1, &arrowBuffers.VAO);
    glGenBuffers(1, &arrowBuffers.VBO);
    glGenBuffers(1, &arrowBuffers.EBO);
    glGenBuffers(2, arrowBuffers.instanceVBOs.data());
    
    auto arrowVertices = Geometry::generateArrowVertices();
    auto arrowIndices = Geometry::generateArrowIndices();
    
    glBindVertexArray(arrowBuffers.VAO);
    
    // Vértices
    glBindBuffer(GL_ARRAY_BUFFER, arrowBuffers.VBO);
    glBufferData(GL_ARRAY_BUFFER, arrowVertices.size() * sizeof(float),
                 arrowVertices.data(), GL_STATIC_DRAW);
    
    // Índices
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, arrowBuffers.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, arrowIndices.size() * sizeof(unsigned int),
                 arrowIndices.data(), GL_STATIC_DRAW);
    
    // Atributos de vértice (posición)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Matrices de instancia (location 4-7)
    glBindBuffer(GL_ARRAY_BUFFER, arrowBuffers.instanceVBOs[0]);
    size_t vec4Size = sizeof(glm::vec4);
    for (int i = 0; i < 4; ++i) {
        glEnableVertexAttribArray(4 + i);
        glVertexAttribPointer(4 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4),
                             (void*)(i * vec4Size));
        glVertexAttribDivisor(4 + i, 1);
    }
    
    // Colores de instancia
    glBindBuffer(GL_ARRAY_BUFFER, arrowBuffers.instanceVBOs[1]);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(3);
    glVertexAttribDivisor(3, 1);
    
    glBindVertexArray(0);
}

void Renderer::updateSphereInstances(const Arcane& arcane) {
    auto positions = arcane.getNodePositions();
    auto colors = arcane.getNodeColors();
    
    if (!positions.empty()) {
        glBindBuffer(GL_ARRAY_BUFFER, sphereBuffers.instanceVBOs[0]);
        glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec3),
                     positions.data(), GL_DYNAMIC_DRAW);
    }
    
    if (!colors.empty()) {
        glBindBuffer(GL_ARRAY_BUFFER, sphereBuffers.instanceVBOs[1]);
        glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(glm::vec3),
                     colors.data(), GL_DYNAMIC_DRAW);
    }
}

void Renderer::updateArrowInstances(const Arcane& arcane) {
    auto transforms = arcane.getArrowTransforms();
    auto colors = arcane.getArrowColors();
    
    if (!transforms.empty()) {
        glBindBuffer(GL_ARRAY_BUFFER, arrowBuffers.instanceVBOs[0]);
        glBufferData(GL_ARRAY_BUFFER, transforms.size() * sizeof(glm::mat4),
                     transforms.data(), GL_DYNAMIC_DRAW);
    }
    
    if (!colors.empty()) {
        glBindBuffer(GL_ARRAY_BUFFER, arrowBuffers.instanceVBOs[1]);
        glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(glm::vec3),
                     colors.data(), GL_DYNAMIC_DRAW);
    }
}

void Renderer::render(const Arcane& arcane) {
    // Limpiar buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Actualizar datos de instancias
    updateSphereInstances(arcane);
    updateArrowInstances(arcane);
    
    auto nodePositions = arcane.getNodePositions();
    auto arrowTransforms = arcane.getArrowTransforms();
    
    // Renderizar flechas si hay datos
    if (!arrowTransforms.empty()) {
        shaderManager->useShader("arrow");
        GLuint arrowProgram = shaderManager->getShaderProgram("arrow");
        
        GLint viewLoc = glGetUniformLocation(arrowProgram, "view");
        GLint projLoc = glGetUniformLocation(arrowProgram, "projection");
        
        if (viewLoc != -1 && projLoc != -1) {
            glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
            glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
        }
        
        glBindVertexArray(arrowBuffers.VAO);
        auto arrowIndices = Geometry::generateArrowIndices();
        glDrawElementsInstanced(GL_TRIANGLES, 
                               static_cast<GLsizei>(arrowIndices.size()),
                               GL_UNSIGNED_INT, 0,
                               static_cast<GLsizei>(arrowTransforms.size()));
        glBindVertexArray(0);
    }
    
    // Renderizar esferas si hay datos
    if (!nodePositions.empty()) {
        shaderManager->useShader("sphere");
        GLuint sphereProgram = shaderManager->getShaderProgram("sphere");
        
        GLint mvpLoc = glGetUniformLocation(sphereProgram, "mvp");
        if (mvpLoc != -1) {
            glm::mat4 mvp = projection * view;
            glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(mvp));
        }
        
        glBindVertexArray(sphereBuffers.VAO);
        auto sphereIndices = Geometry::generateSphereIndices();
        glDrawElementsInstanced(GL_TRIANGLES,
                               static_cast<GLsizei>(sphereIndices.size()),
                               GL_UNSIGNED_INT, 0,
                               static_cast<GLsizei>(nodePositions.size()));
        glBindVertexArray(0);
    }
    
    // Verificar errores de OpenGL
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "OpenGL error: " << error << std::endl;
    }
}

void Renderer::cleanup() {
    sphereBuffers.cleanup();
    arrowBuffers.cleanup();
    
    if (shaderManager) {
        shaderManager->cleanup();
    }
}

glm::mat4 Renderer::calculateProjection(float aspectRatio) noexcept {
    return glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);
}

void Renderer::MeshBuffers::cleanup() noexcept {
    if (VAO) glDeleteVertexArrays(1, &VAO);
    if (VBO) glDeleteBuffers(1, &VBO);
    if (EBO) glDeleteBuffers(1, &EBO);
    for (auto& vbo : instanceVBOs) {
        if (vbo) glDeleteBuffers(1, &vbo);
    }
    VAO = VBO = EBO = 0;
    instanceVBOs.fill(0);
}