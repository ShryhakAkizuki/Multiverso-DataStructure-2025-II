#include "GUI.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <iostream>

void GUI::initialize(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

void GUI::beginFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void GUI::render(Arcane& arcane) {
    renderNetworkControls();
    renderPathFindingControls();
}

void GUI::renderNetworkControls() {
    ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(350, 120), ImGuiCond_Once);
    
    ImGui::Begin("Control de Nodos", nullptr,
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
    
    ImGui::PushItemWidth(100);
    if (ImGui::InputInt("Número de nodos", &newNodeCount)) {
        if (newNodeCount < 36) newNodeCount = 36;
    }
    ImGui::PopItemWidth();
    
    ImGui::PushItemWidth(100);
    if (ImGui::InputInt("Nodos iniciales", &newInitialNodes)) {
        if (newInitialNodes < 2) newInitialNodes = 2;
        if (newInitialNodes > newNodeCount) newInitialNodes = newNodeCount;
    }
    ImGui::PopItemWidth();
    
    if (ImGui::Button("Generar red")) {
        regenerateRequested = true;
    } else {
        regenerateRequested = false;
    }
    
    ImGui::End();
}

void GUI::renderPathFindingControls() {
    ImGui::SetNextWindowPos(ImVec2(20, 150), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(200, 120), ImGuiCond_Once);
    
    ImGui::Begin("Búsqueda de ruta", nullptr,
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
    
    ImGui::PushItemWidth(100);
    if (ImGui::InputInt("Nodo origen", &selectedNode1)) {
        if (selectedNode1 < 0) selectedNode1 = 0;
        if (selectedNode1 >= newNodeCount) selectedNode1 = newNodeCount - 1;
    }
    ImGui::PopItemWidth();
    
    ImGui::PushItemWidth(100);
    if (ImGui::InputInt("Nodo destino", &selectedNode2)) {
        if (selectedNode2 < 0) selectedNode2 = 0;
        if (selectedNode2 >= newNodeCount) selectedNode2 = newNodeCount - 1;
    }
    ImGui::PopItemWidth();
    
    if (ImGui::Button("Buscar ruta")) {
        pathFindingRequested = true;
    } else {
        pathFindingRequested = false;
    }
    
    ImGui::End();
}

void GUI::endFrame() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void GUI::cleanup() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}