#include "ShaderManager.hpp"
#include <iostream>

ShaderManager::ShaderResult ShaderManager::loadShaderProgram(
    std::string_view name, 
    std::string_view vertexSrc, 
    std::string_view fragmentSrc) {
    
    auto vertexResult = compileShader(GL_VERTEX_SHADER, vertexSrc);
    if (!vertexResult.success) {
        return {false, 0, "Vertex shader compilation failed: " + vertexResult.errorMessage};
    }
    
    auto fragmentResult = compileShader(GL_FRAGMENT_SHADER, fragmentSrc);
    if (!fragmentResult.success) {
        glDeleteShader(vertexResult.shaderId);
        return {false, 0, "Fragment shader compilation failed: " + fragmentResult.errorMessage};
    }
    
    auto linkResult = linkProgram(vertexResult.shaderId, fragmentResult.shaderId);
    
    // Limpiar shaders intermedios
    glDeleteShader(vertexResult.shaderId);
    glDeleteShader(fragmentResult.shaderId);
    
    if (linkResult.success) {
        shaderPrograms[std::string(name)] = linkResult.programId;
    }
    
    return linkResult;
}

ShaderManager::CompileResult ShaderManager::compileShader(GLenum type, std::string_view source) {
    GLuint shader = glCreateShader(type);
    const char* sourcePtr = source.data();
    glShaderSource(shader, 1, &sourcePtr, nullptr);
    glCompileShader(shader);
    
    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    
    if (!success) {
        char infoLog[1024];
        glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
        glDeleteShader(shader);
        return {false, 0, std::string(infoLog)};
    }
    
    return {true, shader, ""};
}

ShaderManager::ShaderResult ShaderManager::linkProgram(GLuint vertexShader, GLuint fragmentShader) {
    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    
    int success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    
    if (!success) {
        char infoLog[1024];
        glGetProgramInfoLog(program, 1024, nullptr, infoLog);
        glDeleteProgram(program);
        return {false, 0, std::string(infoLog)};
    }
    
    return {true, program, ""};
}

GLuint ShaderManager::getShaderProgram(std::string_view name) const {
    auto it = shaderPrograms.find(std::string(name));
    return (it != shaderPrograms.end()) ? it->second : 0;
}

void ShaderManager::useShader(std::string_view name) const {
    GLuint program = getShaderProgram(name);
    if (program) {
        glUseProgram(program);
    }
}

void ShaderManager::cleanup() {
    for (auto& [name, program] : shaderPrograms) {
        glDeleteProgram(program);
    }
    shaderPrograms.clear();
}