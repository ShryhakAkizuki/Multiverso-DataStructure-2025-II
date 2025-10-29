#pragma once
#include "imgui.h"
#include <GLFW/glfw3.h> // ¡FALTABA ESTE HEADER!
#include <cstdint>
#include <optional>

class Arcane;

class GUI {
public:
    void initialize(GLFWwindow* window);
    void beginFrame();
    void render(Arcane& arcane);
    void endFrame();
    void cleanup();
    
    // Getters con C++20
    [[nodiscard]] std::pair<int, int> getSelectedNodes() const noexcept { 
        return {selectedNode1, selectedNode2}; 
    }
    [[nodiscard]] std::optional<std::pair<int, int>> getRegenerationParams() const noexcept {
        if (regenerateRequested) 
            return {{newNodeCount, newInitialNodes}};
        return std::nullopt;
    }
    [[nodiscard]] bool isPathFindingRequested() const noexcept { 
        return pathFindingRequested; 
    }
    
private:
    int selectedNode1 = 0;
    int selectedNode2 = 1;
    bool regenerateRequested = false;
    bool pathFindingRequested = false;
    int newNodeCount = 36;
    int newInitialNodes = 2;
    
    void renderNetworkControls();
    void renderPathFindingControls();
};