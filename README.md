# Multiverso-DataStructure-2025-II



![C++ version](https://img.shields.io/badge/C++-17-blue?logo=c%252B%252B)
![OpenGL version](https://img.shields.io/badge/OpenGL-3.3-5586A4?logo=opengl)
![Cmake version](https://img.shields.io/badge/CMake-3.5+-064F8C?logo=cmake)
![Licencia AGPLv3](https://img.shields.io/badge/License-AGPLv3-green?logo=gplv3)
![C++ version](https://img.shields.io/badge/Platform-Windows-blueviolet)

Un visualizador interactivo de redes jerárquicas en 3D implementado en C++ moderno con OpenGL. 

## Estructura del proyecto

``` powershell
Multiverso/
├── core/           # Lógica central de la red
│   ├── Arcane.cpp/hpp    # Clase principal de la red
│   ├── Node.cpp/hpp      # Nodos con conexiones input/output
│   ├── Arrow.cpp/hpp     # Flechas con transformaciones 3D
│   └── DynamicArray.hpp  # Contenedor personalizado tipo vector
│
├── graphics/       # Renderizado OpenGL
│   ├── Renderer.cpp/hpp        # Sistema principal de renderizado
│   ├── ShaderManager.cpp/hpp   # Gestión de shaders
│   └── Geometry.cpp/hpp        # Generación de mallas 3D
│
├── utils/          # Utilidades
│   ├── Camera.cpp/hpp    # Cámara orbital 3D
│   ├── MathUtils.hpp     # Funciones matemáticas avanzadas
│   └── InputHandler.hpp  # Manejo de input (GLFW)
│
├── ui/             # Interfaz de usuario
│   └── GUI.cpp/hpp       # ImGui integration
│
└── main.cpp        # Punto de entrada
```

## Tecnologías utilizadas


|Dependencia	|Versión	|Propósito	        |FetchContent Automático|
|---------------|-----------|-------------------|-----------------------|
|GLFW	        |3.3.10	    |Windowing y input	|           ✅          |
|GLAD	        |v0.1.36	|Loader de OpenGL	|           ✅          |
|GLM	        |0.9.9.8	|Matemáticas 3D	    |           ✅          |
|Dear ImGui	    |v1.89.8	|Interfaz de usuario|           ✅          |


## Controles

|control                        |acción                         |      
|-------------------------------|-------------------------------|
|Click izquierdo + arrastrar	|Rotar cámara                   |
|Rueda del mouse	            |Zoom in/out                    |
|UI: Número de nodos	        |Controlar tamaño de red        |
|UI: Nodos iniciales	        |Controlar jerarquía inicial    |
|UI: Buscar ruta	            |Encontrar camino entre nodos   |
