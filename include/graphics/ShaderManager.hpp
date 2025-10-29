#pragma once
#include <glad/glad.h>
#include <string>
#include <unordered_map>
#include <string_view>
#include <optional> // Usar std::optional en lugar de std::expected

class ShaderManager {
public:
    ShaderManager() = default;
    ~ShaderManager() { cleanup(); }
    
    // Reemplazar std::expected con std::optional + string de error
    struct ShaderResult {
        bool success;
        GLuint programId;
        std::string errorMessage;
    };
    
    ShaderResult loadShaderProgram(
        std::string_view name, 
        std::string_view vertexSrc, 
        std::string_view fragmentSrc);
    
    [[nodiscard]] GLuint getShaderProgram(std::string_view name) const;
    void useShader(std::string_view name) const;
    void cleanup();
    
private:
    std::unordered_map<std::string, GLuint> shaderPrograms;
    
    struct CompileResult {
        bool success;
        GLuint shaderId;
        std::string errorMessage;
    };
    
    [[nodiscard]] CompileResult compileShader(GLenum type, std::string_view source);
    [[nodiscard]] ShaderResult linkProgram(GLuint vertexShader, GLuint fragmentShader);
};