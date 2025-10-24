cmake_minimum_required(VERSION 3.5)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
project(Multiverso CXX)

# --- Configuración general ---
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# --- Limpieza automática de build antes de compilar ---
add_custom_target(clean_all
    COMMAND ${CMAKE_COMMAND} -E rm -rf ${CMAKE_BINARY_DIR}/*
    COMMENT "Limpiando compilación anterior..."
)
add_custom_target(rebuild
    COMMAND ${CMAKE_COMMAND} --build . --target clean_all
    COMMAND ${CMAKE_COMMAND} --build .
    COMMENT "Reconstruyendo proyecto completo..."
)

# --- Archivos fuente ---
file(GLOB_RECURSE SOURCES "src/*.cpp" "include/*.hpp")
add_executable(${PROJECT_NAME} ${SOURCES})

# --- Traer Open3D con FetchContent ---
include(FetchContent)

# --- OpenGL ---
find_package(OpenGL REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE OpenGL::GL)

# --- FetchContent ---
include(FetchContent)

# ----- GLFW -----
FetchContent_Declare( 
    glfw 
    GIT_REPOSITORY 
    https://github.com/glfw/glfw.git 
    GIT_TAG 3.3.10 ) 
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "") 
set(GLFW_BUILD_TESTS OFF CACHE BOOL "") 
set(GLFW_BUILD_DOCS OFF CACHE BOOL "") 

FetchContent_MakeAvailable(glfw) 

target_link_libraries(${PROJECT_NAME} PRIVATE glfw) 

# ----- GLAD ----- 
FetchContent_Declare( 
    glad 
    GIT_REPOSITORY 
    https://github.com/Dav1dde/glad.git 
    GIT_TAG v0.1.36 ) 
FetchContent_MakeAvailable(glad) 

target_link_libraries(${PROJECT_NAME} PRIVATE glad) 

# ----- GLM ----- 
FetchContent_Declare( 
    glm 
    GIT_REPOSITORY 
    https://github.com/g-truc/glm.git 
    GIT_TAG 0.9.9.8 ) 
FetchContent_MakeAvailable(glm) 

target_link_libraries(${PROJECT_NAME} PRIVATE glm::glm)

# ----- ImGui -----
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.89.8  # Última versión estable en C++
)
FetchContent_MakeAvailable(imgui)

# --- Integración ImGui + GLFW + OpenGL3 ---
target_sources(${PROJECT_NAME} PRIVATE
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

target_link_libraries(${PROJECT_NAME} PRIVATE glfw glad)

# --- Include path ---
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/include)

# --- Mensaje de estado ---
message(STATUS "Configuración lista: OpenGL + GLFW + GLAD + GLM")
message(STATUS "Ejecutable se generará en: ${EXECUTABLE_OUTPUT_PATH}")