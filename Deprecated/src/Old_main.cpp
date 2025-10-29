#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>   
#include <glm/gtx/string_cast.hpp>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <vector>
#include <iostream>
#include <random>
#include <cmath>
#include <numbers>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <cstdint>
#include <algorithm>


// --- Variables globales  ---
float yaw = 90.0f, pitch = 0.0f;
float lastX = 400.0f, lastY = 300.0f;
bool firstMouse = true;
float radius = 8.0f;

// --- Shaders (mínimos) ---
// arrow: instanceColor location=3, instanceMatrix location=4 (4..7)
const char* arrowVert = R"(
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
const char* arrowFrag = R"(
#version 330 core
in vec3 fragColor;
out vec4 FragColor;
void main() {
    FragColor = vec4(fragColor, 1.0);
}
)";

// sphere: aPos location=0, instancePos location=2, instanceColor location=3
const char* sphereVert = R"(
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
const char* sphereFrag = R"(
#version 330 core
in vec3 vColor;
out vec4 FragColor;
void main() {
    FragColor = vec4(vColor, 1.0);
}
)";
// --- Geometria ---

std::vector<unsigned int> getSphereIndices(unsigned int stacks = 16, unsigned int sectors = 32){
    std::vector<unsigned int> indices;

    for (unsigned int i = 0; i < stacks; ++i) {
        unsigned int k1 = i * (sectors + 1);
        unsigned int k2 = k1 + sectors + 1;

        for (unsigned int j = 0; j < sectors; ++j, ++k1, ++k2) {
            if (i != 0) indices.insert(indices.end(), {k1, k2, k1 + 1});
            if (i != stacks - 1) indices.insert(indices.end(), {k1 + 1, k2, k2 + 1});
        }
    }

    return indices;
}

std::vector<float> getSphereVertices(float r = 0.2f, unsigned int stacks = 16, unsigned int sectors = 32){
    std::vector<float> vertices;

    for (unsigned int i = 0; i <= stacks; ++i) {
        float stackAngle = static_cast<float>(std::numbers::pi / 2 - i * std::numbers::pi / stacks);
        float xy = r * cosf(stackAngle);
        float z = r * sinf(stackAngle);

        for (unsigned int j = 0; j <= sectors; ++j) {
            float sectorAngle = static_cast<float>(j * 2 * std::numbers::pi / sectors);
            float x = xy * cosf(sectorAngle);
            float y = xy * sinf(sectorAngle);
            vertices.insert(vertices.end(), {x, y, z});
        }
    }
    return vertices;
}

std::vector<unsigned int> getArrowIndices(unsigned int segments = 16){
    std::vector<unsigned int> indices;
    // cuerpo cilindro (each ring has 2 verts per segment)
    for (unsigned int i = 0; i < segments; ++i) {
        unsigned int base = i * 2;
        indices.insert(indices.end(), {
            base, base + 1, base + 3,
            base, base + 3, base + 2
        });
    }
    // base del cono: we assume base ring starts at offset
    unsigned int offset = (segments + 1) * 2;
    for (unsigned int i = 0; i < segments; ++i) {
        unsigned int a = offset + i;
        unsigned int b = offset + (i + 1) % (segments + 1);
        unsigned int apex = offset + (segments + 1); // last vertex is apex
        indices.insert(indices.end(), { a, b, apex });
    }
    return indices;
}

std::vector<float> getArrowVertices(float shaftLength = 0.8f, float shaftRadius = 0.02f, float headLength  = 0.2f, float headRadius  = 0.05f,unsigned int segments = 16){
    std::vector<float> vertices;
    const float PI = std::numbers::pi_v<float>;

    // cilindro: for i in [0..segments] push (x*R, y*R, z=0) and (x*R, y*R, z=shaftLength)
    for (unsigned int i = 0; i <= segments; ++i) {
        float theta = 2.0f * PI * i / segments;
        float x = cosf(theta);
        float y = sinf(theta);
        vertices.insert(vertices.end(), { x * shaftRadius, y * shaftRadius, 0.0f });
        vertices.insert(vertices.end(), { x * shaftRadius, y * shaftRadius, shaftLength });
    }

    // cono base ring
    for (unsigned int i = 0; i <= segments; ++i) {
        float theta = 2.0f * PI * i / segments;
        float x = cosf(theta);
        float y = sinf(theta);
        vertices.insert(vertices.end(), { x * headRadius, y * headRadius, shaftLength });
    }
    // apex
    vertices.insert(vertices.end(), { 0.0f, 0.0f, shaftLength + headLength });

    return vertices;
}

// --- Callbacks ---
void framebuffer_size_callback(GLFWwindow*, int w, int h) {
    glViewport(0, 0, w, h);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    // Si el botón izquierdo NO está presionado, no rotamos y preparamos
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) != GLFW_PRESS) {
        firstMouse = true; // sincronizar para evitar saltos al volver a presionar
        return;
    }

    // Si es la primera vez que movemos con el botón presionado, inicializamos la referencia
    if (firstMouse) {
        lastX = static_cast<float>(xpos);
        lastY = static_cast<float>(ypos);
        firstMouse = false;
    }

    float xoffset = static_cast<float>(xpos) - lastX;
    float yoffset = lastY - static_cast<float>(ypos); // invertir Y para comportamiento típico
    lastX = static_cast<float>(xpos);
    lastY = static_cast<float>(ypos);

    const float sens = 0.2f;
    yaw += xoffset * sens;
    pitch += yoffset * sens;

    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;
}

// mouse button: captura/soltar cursor al presionar/soltar el botón izquierdo
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button != GLFW_MOUSE_BUTTON_LEFT) return;

    if (action == GLFW_PRESS) {
        // Capturar y ocultar cursor; la rotación comienza (mouse_callback leerá PRESSED)
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        firstMouse = true; // forzar re-sincronización de la posición la primera vez
    } else if (action == GLFW_RELEASE) {
        // Restaurar cursor visible/normal; rotación se detiene
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        firstMouse = true; // prevenir saltos en la siguiente captura
    }
}

void scroll_callback(GLFWwindow*, double, double yoffset) {
    radius -= static_cast<float>(yoffset) * 0.5f;
    if (radius < 2.0f) radius = 2.0f;
    if (radius > 50.0f) radius = 50.0f;
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// --- Utilidad: compilar shaders ---
GLuint compileShader(GLenum type, const char* src) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);
    int success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char info[1024];
        glGetShaderInfoLog(shader, 1024, nullptr, info);
        std::cerr << "Error compilando shader: " << info << std::endl;
    }
    return shader;
}

GLuint createProgram(const char* vSrc, const char* fSrc) {
    GLuint v = compileShader(GL_VERTEX_SHADER, vSrc);
    GLuint f = compileShader(GL_FRAGMENT_SHADER, fSrc);
    GLuint program = glCreateProgram();
    glAttachShader(program, v);
    glAttachShader(program, f);
    glLinkProgram(program);

    int success = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char info[1024];
        glGetProgramInfoLog(program, 1024, nullptr, info);
        std::cerr << "Error linking program: " << info << std::endl;
    }

    glDeleteShader(v);
    glDeleteShader(f);
    return program;
}

// --- Clases ---

struct Node {
  uint32_t id = -1;          // Identificador unico
  uint32_t level = -1;       // Nivel de la jerarquia
  uint8_t conexiones = 0;   // Numero de conexiones
  uint8_t conexiones_input = 0;        // Numero de conexiones - entrada
  uint8_t conexiones_output = 0;       // Numero de conexiones - salida

  glm::vec3 posicion = glm::vec3(0.0f);   // vector 3D de tipo float
  glm::vec3 color = glm::vec3(1.0f);;
  
  Node **output = nullptr;        // Referencia a los nodos a los que esta conectado este nodo
  Node **input = nullptr;         // Referencia a los nodos que conectan a este nodo
};

struct Arrow{
    const Node* origen = nullptr;
    const Node* destino = nullptr;
    glm::mat4 Posicion = glm::mat4(1.0f);
    glm::vec3 color = glm::vec3(1.0f);
};

class Arcane{
private:
    Node* nodos  = nullptr;
    std::vector<Arrow> flechas;

    uint32_t numNodos = 0;
    uint32_t niveles = 0;
    std::mt19937 gen;

public:
    Arcane();
    Arcane(uint32_t numNodosParam, uint32_t Nodos_Iniciales);
    ~Arcane();

    Arcane(const Arcane&) = delete;
    Arcane& operator=(const Arcane&) = delete;

    Arcane(Arcane&& other) noexcept;
    Arcane& operator=(Arcane&& other) noexcept;

    
    uint32_t getNumLevels() const { return niveles + 1; }
    std::vector<const Node*> ruta(uint32_t id_origen, uint32_t id_destino);
    std::vector<glm::vec3> ruta_flechas(const std::vector<const Node*>& path, glm::vec3 highlightColor); 
    
    std::vector<glm::mat4> getArrowTransforms();
    std::vector<glm::vec3> getArrowColors();
    std::vector<glm::vec3> getNodePositions();
    std::vector<glm::vec3> getNodeColors();
    
    void AsignarColorFlechas();

private:

    void InicializarNodos(uint32_t Inicial = 2);
    bool ConexionSegura(Node* origen, Node* destino);
    bool ConexionForzada(Node* origen, Node* destino);
    void ConectarNodos();
    void Asignar3D();
    void AsignarColorNivel();
    void GenerarFlechas();
    void ActualizarFlecha(Arrow& flecha, float sphereRadius = 0.2f, float thickness = 1.0f);

};

Arcane::Arcane(): numNodos(36) {
    std::random_device rd;
    gen.seed(rd());

    nodos = new Node[numNodos];
    for (uint32_t i = 0; i < numNodos; ++i) {
        nodos[i].input = nullptr;
        nodos[i].output = nullptr;
    }

    InicializarNodos();
    ConectarNodos();
    Asignar3D();
    AsignarColorNivel();
    GenerarFlechas();
    AsignarColorFlechas();
}

Arcane::Arcane(uint32_t numNodosParam, uint32_t Nodos_Iniciales) {
    numNodos = std::max<uint32_t>(36u, numNodosParam);
    std::random_device rd;
    gen.seed(rd());
    nodos = new Node[numNodos];
    InicializarNodos(Nodos_Iniciales);
    ConectarNodos();
    Asignar3D();
    AsignarColorNivel();
    GenerarFlechas();
    AsignarColorFlechas();
}

Arcane::~Arcane() {
    if (nodos) {
        for (uint32_t i = 0; i < numNodos; ++i) {
            delete[] nodos[i].input;
            delete[] nodos[i].output;
        }
        delete[] nodos;
    }
}

Arcane::Arcane(Arcane&& other) noexcept
    : nodos(other.nodos),
      flechas(std::move(other.flechas)),
      numNodos(other.numNodos),
      niveles(other.niveles),
      gen(std::move(other.gen)) {
    other.nodos = nullptr;
    other.numNodos = 0;
    other.niveles = 0;
}

Arcane& Arcane::operator=(Arcane&& other) noexcept {
    if (this != &other) {
        if (nodos) {
            for (uint32_t i = 0; i < numNodos; ++i) {
                delete[] nodos[i].input;
                delete[] nodos[i].output;
            }
            delete[] nodos;
        }
        nodos = other.nodos;
        flechas = std::move(other.flechas);
        numNodos = other.numNodos;
        niveles = other.niveles;
        gen = std::move(other.gen);
        other.nodos = nullptr;
        other.numNodos = 0;
        other.niveles = 0;
    }
    return *this;
}

void Arcane::InicializarNodos(uint32_t Inicial) {
    if (Inicial < 2) Inicial = 2;
    uint32_t limite = Inicial, acumulado = limite;
    std::uniform_int_distribution<int> distConn(2, 6);
    std::uniform_int_distribution<int> distInputs;

    niveles = 0;
    for (uint32_t i = 0; i < numNodos; ++i) {
        Node& n = nodos[i];
        n.id = i;
        n.level = niveles;
        if (i >= acumulado - 1) {
            ++niveles;
            limite *= 2;
            acumulado += limite;
        }
    }

    for (uint32_t i = 0; i < numNodos; ++i) {
        Node& n = nodos[i];
        n.conexiones = static_cast<uint8_t>(distConn(gen));
        uint8_t maxIn = std::max<uint8_t>(1, n.conexiones - 1);
        distInputs.param(std::uniform_int_distribution<int>::param_type(1, maxIn));
        n.conexiones_input = static_cast<uint8_t>(distInputs(gen));
        n.conexiones_output = n.conexiones - n.conexiones_input;
        n.input = new Node*[n.conexiones_input]();
        n.output = new Node*[n.conexiones_output]();
    }
}

bool Arcane::ConexionSegura(Node* origen, Node* destino) {
    if (!origen || !destino || origen == destino) return false;
    for (uint8_t i = 0; i < origen->conexiones_output; ++i)
        if (origen->output[i] == destino) return false;
    
    for (uint8_t i = 0; i < destino->conexiones_output; ++i)
        if (destino->output[i] == origen) return false;
    
    for (uint8_t i = 0; i < origen->conexiones_output; ++i)
        if (!origen->output[i]) {
            for (uint8_t j = 0; j < destino->conexiones_input; ++j)
                if (!destino->input[j]) {
                    origen->output[i] = destino;
                    destino->input[j] = origen;
                    return true;
                }
        }
    return false;
}

bool Arcane::ConexionForzada(Node* origen, Node* destino) {
    if (!origen || !destino || origen == destino) return false;
    const uint8_t MAX_CONN = 6;
    if (origen->conexiones < MAX_CONN) {
        Node** nuevoOut = new Node*[origen->conexiones_output + 1]();
        std::copy(origen->output, origen->output + origen->conexiones_output, nuevoOut);
        delete[] origen->output;
        origen->output = nuevoOut;
        origen->conexiones_output++;
        origen->conexiones++;
    }
    if (destino->conexiones < MAX_CONN) {
        Node** nuevoIn = new Node*[destino->conexiones_input + 1]();
        std::copy(destino->input, destino->input + destino->conexiones_input, nuevoIn);
        delete[] destino->input;
        destino->input = nuevoIn;
        destino->conexiones_input++;
        destino->conexiones++;
    }
    return ConexionSegura(origen, destino);
}

void Arcane::ConectarNodos() {
    const uint32_t MAX_INTENTOS = 8;
    const uint8_t MAX_CONN = 6;

    Node*** NivelesNodos = new Node**[niveles + 1];
    uint32_t* ConteoNodosNivel = new uint32_t[niveles + 1]{0};
    uint32_t* IndicesNivel = new uint32_t[niveles + 1]{0};

    std::uniform_real_distribution<float> prob(0.0f, 1.0f);

    // Contar por nivel
    for (uint32_t i = 0; i < numNodos; ++i) ++ConteoNodosNivel[nodos[i].level];
    for (uint32_t l = 0; l <= niveles; ++l) NivelesNodos[l] = new Node*[ConteoNodosNivel[l]];
    for (uint32_t i = 0; i < numNodos; ++i) {
        uint32_t nivel = nodos[i].level;
        NivelesNodos[nivel][IndicesNivel[nivel]++] = &nodos[i];
    }

    // Fase 1: intentar crear las conexiones planificadas
    for (uint32_t i = 0; i < numNodos; ++i) {
        Node& actual = nodos[i];
        uint32_t nivel = actual.level;
        uint8_t wanted = actual.conexiones_output;
        uint8_t made = 0;

        for (uint8_t c = 0; c < wanted; ++c) {
            bool conectado = false;
            uint32_t intentos = 0;
            while (!conectado && intentos < MAX_INTENTOS) {
                ++intentos;
                uint32_t nivelDestino = nivel;
                float p = prob(gen);
                if (nivel == 0) nivelDestino = (p < 0.333f) ? nivel : nivel + 1;
                else if (nivel == niveles) nivelDestino = nivel - 1;
                else {
                    if (p < 0.25f) nivelDestino = nivel - 1;
                    else if (p < 0.75f) nivelDestino = nivel;
                    else nivelDestino = nivel + 1;
                }
                if (nivelDestino > niveles) nivelDestino = niveles;
                uint32_t nd = ConteoNodosNivel[nivelDestino];
                if (nd == 0) continue;
                std::uniform_int_distribution<uint32_t> pick(0, nd - 1);
                Node* destino = NivelesNodos[nivelDestino][pick(gen)];
                if (!destino || destino == &actual) continue;

                // Evitar bidireccionalidad rápida: si destino ya apunta a actual, saltar
                bool destPointsToActual = false;
                for (uint8_t k = 0; k < destino->conexiones_output; ++k)
                    if (destino->output[k] == &actual) { destPointsToActual = true; break; }
                if (destPointsToActual) continue;

                conectado = ConexionSegura(&actual, destino);
            }

            // si no se conectó con intentos normales, forzar una vez
            if (!conectado) {
                // pick un destino forzado (intentar varios destinos antes de rendirse)
                bool forced = false;
                for (uint32_t tries = 0; tries < 4 && !forced; ++tries) {
                    uint32_t nivelDestino = nivel;
                    float p = prob(gen);
                    if (nivel == 0) nivelDestino = (p < 0.333f) ? nivel : nivel + 1;
                    else if (nivel == niveles) nivelDestino = nivel - 1;
                    else {
                        if (p < 0.25f) nivelDestino = nivel - 1;
                        else if (p < 0.75f) nivelDestino = nivel;
                        else nivelDestino = nivel + 1;
                    }
                    if (nivelDestino > niveles) nivelDestino = niveles;
                    uint32_t nd = ConteoNodosNivel[nivelDestino];
                    if (nd == 0) continue;
                    std::uniform_int_distribution<uint32_t> pick(0, nd - 1);
                    Node* destino = NivelesNodos[nivelDestino][pick(gen)];
                    if (!destino || destino == &actual) continue;
                    // intentar forzar
                    if (ConexionForzada(&actual, destino)) { forced = true; break; }
                }
                if (!forced) {
                    // fallback: intentar cualquier nodo del grafo por if all else fails
                    for (uint32_t j = 0; j < numNodos && !forced; ++j) {
                        if (&nodos[j] == &actual) continue;
                        if (ConexionForzada(&actual, &nodos[j])) forced = true;
                    }
                }
            }
        }
    }

    // Fase 2: asegurar que cada nodo tenga al menos 1 output y 1 input (verificando slots ocupados)
    for (uint32_t i = 0; i < numNodos; ++i) {
        Node& n = nodos[i];

        // contar outputs ocupados reales
        uint8_t filledOut = 0;
        for (uint8_t k = 0; k < n.conexiones_output; ++k) if (n.output[k] != nullptr) ++filledOut;

        if (filledOut == 0) {
            // intentar conectar hacia niveles preferidos: nivel+1, mismo, nivel-1
            int pref[3] = { (int)n.level + 1, (int)n.level, (int)n.level - 1 };
            for (int pi = 0; pi < 3 && filledOut == 0; ++pi) {
                int lvl = pref[pi];
                if (lvl < 0 || lvl > (int)niveles) continue;
                uint32_t nd = ConteoNodosNivel[lvl];
                for (uint32_t t = 0; t < nd && filledOut == 0; ++t) {
                    Node* dest = NivelesNodos[lvl][t];
                    if (dest == &n) continue;
                    if (ConexionForzada(&n, dest)) {
                        ++filledOut;
                        break;
                    }
                }
            }
            // último recurso: cualquier nodo
            for (uint32_t j = 0; j < numNodos && filledOut == 0; ++j) {
                if (&nodos[j] == &n) continue;
                if (ConexionForzada(&n, &nodos[j])) { ++filledOut; break; }
            }
        }

        // contar inputs ocupados reales
        uint8_t filledIn = 0;
        for (uint8_t k = 0; k < n.conexiones_input; ++k) if (n.input[k] != nullptr) ++filledIn;

        if (filledIn == 0) {
            // intentar desde niveles preferidos: nivel-1, mismo, nivel+1
            int pref[3] = { (int)n.level - 1, (int)n.level, (int)n.level + 1 };
            for (int pi = 0; pi < 3 && filledIn == 0; ++pi) {
                int lvl = pref[pi];
                if (lvl < 0 || lvl > (int)niveles) continue;
                uint32_t nd = ConteoNodosNivel[lvl];
                for (uint32_t t = 0; t < nd && filledIn == 0; ++t) {
                    Node* src = NivelesNodos[lvl][t];
                    if (src == &n) continue;
                    if (ConexionForzada(src, &n)) { ++filledIn; break; }
                }
            }
            // último recurso: cualquier nodo
            for (uint32_t j = 0; j < numNodos && filledIn == 0; ++j) {
                if (&nodos[j] == &n) continue;
                if (ConexionForzada(&nodos[j], &n)) { ++filledIn; break; }
            }
        }
    }

    // Fase 3: reforzar último nivel (asegurar entradas)
    if (niveles > 0) {
        uint32_t nivelOrigen = niveles - 1;
        uint32_t ndOrigen = ConteoNodosNivel[nivelOrigen];
        uint32_t ndDest = ConteoNodosNivel[niveles];
        for (uint32_t j = 0; j < ndDest; ++j) {
            Node* destino = NivelesNodos[niveles][j];
            // contar inputs reales
            uint8_t filledIn = 0;
            for (uint8_t k = 0; k < destino->conexiones_input; ++k) if (destino->input[k] != nullptr) ++filledIn;
            if (filledIn == 0) {
                bool connected = false;
                // intentar desde penúltimo nivel
                for (uint32_t k = 0; k < ndOrigen && !connected; ++k) {
                    Node* origen = NivelesNodos[nivelOrigen][k];
                    if (origen == destino) continue;
                    if (ConexionForzada(origen, destino)) connected = true;
                }
                // fallback: cualquier nodo
                for (uint32_t x = 0; x < numNodos && !connected; ++x) {
                    Node* origen = &nodos[x];
                    if (origen == destino) continue;
                    if (ConexionForzada(origen, destino)) connected = true;
                }
            }
        }
    }

    // Limpieza
    for (uint32_t l = 0; l <= niveles; ++l) delete[] NivelesNodos[l];
    delete[] NivelesNodos;
    delete[] ConteoNodosNivel;
    delete[] IndicesNivel;
}

void Arcane::Asignar3D(){
    
    uint32_t* ConteoNodosNivel = new uint32_t[niveles + 1]{0};
    for (uint32_t i = 0; i < numNodos; ++i) {
        if (nodos[i].level <= niveles)
            ++ConteoNodosNivel[nodos[i].level];
    }

    uint32_t* indiceNivel = new uint32_t[niveles + 1]{0};

    for (uint32_t i = 0; i < numNodos; ++i) {
        Node& n = nodos[i];
        uint32_t nivel = n.level;
        uint32_t total = (nivel <= niveles) ? ConteoNodosNivel[nivel] : 0;

        if (total == 0) {
            // Sitio seguro: no hay nodos en este nivel (no debería pasar si inicializaste bien),
            // colocamos en origen temporalmente
            n.posicion = glm::vec3(0.0f);
            continue;
        }

        uint32_t index = indiceNivel[nivel]++;
        float r = static_cast<float>(std::pow(nivel + 1, 2));

        float offset = 2.0f / static_cast<float>(total); // total>0 garantizado
        float y = ((index * offset) - 1.0f) + (offset / 2.0f);

        // Clamp y para evitar pequeños errores numéricos que hagan sqrt(-eps)
        if (y > 1.0f) y = 1.0f;
        if (y < -1.0f) y = -1.0f;

        float radius = sqrt(std::max(0.0f, 1.0f - y * y));

        float phi = index * glm::pi<float>() * (3.0f - sqrt(5.0f)); // ángulo dorado

        float x = radius * cos(phi);
        float z = radius * sin(phi);

        n.posicion = glm::vec3(x, y, z) * r;
    }

    delete[] ConteoNodosNivel;
    delete[] indiceNivel;
}

void Arcane::AsignarColorNivel(){

    const double saturacion = 0.8f;
    const double value = 1.0f;
    const double c = value * saturacion;
    const double m = value - c;
    double r,g,b;

    for(uint32_t i = 0; i<numNodos; ++i){
        Node& n = nodos[i];
        double hue = static_cast<float>(n.level)/(niveles+1);
        double x = c*(1-fabs(fmod(hue*6,2)-1));

        if (hue < 1.0f / 6) { r = c; g = x; b = 0; }
        else if (hue < 2.0f / 6) { r = x; g = c; b = 0; }
        else if (hue < 3.0f / 6) { r = 0; g = c; b = x; }
        else if (hue < 4.0f / 6) { r = 0; g = x; b = c; }
        else if (hue < 5.0f / 6) { r = x; g = 0; b = c; }
        else { r = c; g = 0; b = x; }

        n.color = glm::vec3(r + m, g + m, b + m);
    }
}

std::vector<glm::vec3> Arcane::getNodePositions(){
    std::vector<glm::vec3> pos;
    pos.reserve(numNodos);
    for(uint32_t i = 0; i<numNodos; ++i){
        pos.push_back(nodos[i].posicion);
    }
    return pos;
}

std::vector<glm::vec3> Arcane::getNodeColors(){
    std::vector<glm::vec3> col;
    col.reserve(numNodos);
    for(uint32_t i = 0; i<numNodos; ++i){
        col.push_back(nodos[i].color);
    }
    return col;
}

void Arcane::GenerarFlechas(){
    flechas.clear(); // vaciamos cualquier flecha anterior

    for(uint32_t i = 0; i < numNodos; ++i) {
        Node* nodoOrigen = &nodos[i];

        for(uint32_t j = 0; j < nodoOrigen->conexiones_output; ++j) {
            Node* nodoDestino = nodoOrigen->output[j];
            if (nodoDestino == nullptr) continue;

            Arrow flecha;
            flecha.origen = nodoOrigen;
            flecha.destino = nodoDestino;
            flecha.Posicion = glm::mat4(1.0f); 
            ActualizarFlecha(flecha);
            flechas.push_back(flecha);
        }
    }
}

void Arcane::AsignarColorFlechas() {
    const float oscuridad = 0.7f; 
    for (auto& f : flechas) {
        if (f.origen) {
            f.color = f.origen->color * oscuridad;
        } else {
            f.color = glm::vec3(0.4f, 0.4f, 0.4f);
        }
        f.color = glm::clamp(f.color, 0.0f, 1.0f);
    }
}

void Arcane::ActualizarFlecha(Arrow& flecha, float sphereRadius, float thickness) {
    if (!flecha.origen || !flecha.destino) return;

    glm::vec3 origStart = flecha.origen->posicion;
    glm::vec3 origEnd   = flecha.destino->posicion;

    glm::vec3 dirFull = origEnd - origStart;
    float fullLen = glm::length(dirFull);
    if (fullLen < 1e-5f) {
        flecha.Posicion = glm::mat4(1.0f);
        return;
    }

    glm::vec3 dirNormFull = glm::normalize(dirFull);

    // Ajuste por radios de las esferas
    glm::vec3 start = origStart + dirNormFull * sphereRadius;
    glm::vec3 end   = origEnd   - dirNormFull * sphereRadius;

    glm::vec3 dir = end - start;
    float length = glm::length(dir);
    if (length < 1e-5f) {
        flecha.Posicion = glm::mat4(1.0f);
        return;
    }

    glm::vec3 dirNorm = glm::normalize(dir);

    // Rotación base Z->Y
    glm::mat4 baseRotation = glm::rotate(glm::mat4(1.0f), -glm::half_pi<float>(), glm::vec3(1,0,0));

    // Rotación real para alinear +Y con dirNorm
    glm::vec3 baseDir(0.0f, 1.0f, 0.0f);
    glm::vec3 axis = glm::cross(baseDir, dirNorm);
    float cosTheta = glm::dot(baseDir, dirNorm);
    float angle = acos(glm::clamp(cosTheta, -1.0f, 1.0f));

    glm::mat4 rotation(1.0f);
    if (glm::length(axis) > 1e-6f)
        rotation = glm::rotate(glm::mat4(1.0f), angle, glm::normalize(axis));

    // Escala y traslación
    glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(thickness, length, thickness));
    glm::mat4 translation = glm::translate(glm::mat4(1.0f), start);

    flecha.Posicion = translation * rotation * scale * baseRotation;
}

std::vector<glm::mat4> Arcane::getArrowTransforms(){
    std::vector<glm::mat4> arrowTransforms;

    arrowTransforms.reserve(flechas.size()); 

    for (const auto& f : flechas) {
        arrowTransforms.push_back(f.Posicion);
    }

    return arrowTransforms;
}

std::vector<glm::vec3> Arcane::getArrowColors(){
    std::vector<glm::vec3> arrowColors;

    arrowColors.reserve(flechas.size()); 

    for (const auto& f : flechas) {
        arrowColors.push_back(f.color);
    }

    return arrowColors;
}

std::vector<const Node*> Arcane::ruta(uint32_t id_origen, uint32_t id_destino){
    // Validar ids
    if (id_origen >= numNodos || id_destino >= numNodos) return {};

    // Caso trivial
    if (id_origen == id_destino) {
        return { &nodos[id_origen] };
    }

    std::vector<int> parent(numNodos, -1);
    std::vector<char> visited(numNodos, 0);
    std::queue<uint32_t> q;

    visited[id_origen] = 1;
    q.push(id_origen);

    bool encontrado = false;
    while (!q.empty() && !encontrado) {
        uint32_t u = q.front(); q.pop();
        Node& nodoU = nodos[u];

        // iterar salidas
        for (uint8_t k = 0; k < nodoU.conexiones_output; ++k) {
            Node* vecinoPtr = nodoU.output[k];
            if (!vecinoPtr) continue;
            uint32_t v = vecinoPtr->id;
            if (!visited[v]) {
                visited[v] = 1;
                parent[v] = static_cast<int>(u);
                if (v == id_destino) { encontrado = true; break; }
                q.push(v);
            }
        }
    }

    // Si no se encontró destino
    if (!visited[id_destino]) return {};

    // reconstruir ruta (de destino a origen)
    std::vector<const Node*> ruta;
    int cur = static_cast<int>(id_destino);
    while (cur != -1) {
        ruta.push_back(&nodos[cur]);
        cur = parent[cur];
    }
    std::reverse(ruta.begin(), ruta.end());
    return ruta;
}

std::vector<glm::vec3> Arcane::ruta_flechas(const std::vector<const Node*>& path, glm::vec3 highlightColor) {
    std::vector<glm::vec3> colors;
    colors.reserve(flechas.size());

    // Si la ruta tiene menos de 2 nodos, no hay aristas a destacar
    if (path.size() < 2) {
        // devolver colores actuales (sin cambios)
        for (size_t i = 0; i < flechas.size(); ++i) {
            colors.push_back(flechas[i].color);
        }
        return colors;
    }

    // Construir conjunto de aristas (id_origen,id_destino) presentes en la ruta
    // codificamos par (u,v) como uint64_t = (uint64_t(u) << 32) | v
    std::unordered_set<uint64_t> pathEdges;
    pathEdges.reserve(path.size());
    for (size_t i = 0; i + 1 < path.size(); ++i) {
        const Node* a = path[i];
        const Node* b = path[i+1];
        if (!a || !b) continue;
        uint64_t key = (static_cast<uint64_t>(a->id) << 32) | static_cast<uint64_t>(b->id);
        pathEdges.insert(key);
    }

    // Para cada flecha, comprobar si su (origen->destino) está en pathEdges.
    // Si sí: asignar highlightColor (y devolverlo). Si no: devolver su color actual.
    for (size_t i = 0; i < flechas.size(); ++i) {
        Arrow& f = flechas[i];
        glm::vec3 finalColor = f.color; // por defecto su color actual

        if (f.origen && f.destino) {
            uint64_t key = (static_cast<uint64_t>(f.origen->id) << 32) | static_cast<uint64_t>(f.destino->id);
            if (pathEdges.find(key) != pathEdges.end()) {
                f.color = highlightColor; // actualiza la flecha internamente
                finalColor = f.color;
            } else {
                // Si quieres restaurar colores originales aquí, usa f.originalColor (si lo guardas).
                // Por ahora mantenemos el color existente.
            }
        }
        colors.push_back(finalColor);
    }

    return colors;
}

// --- MAIN ---
int xd() {
    // --- Inicialización de GLFW y OpenGL ---
    if (!glfwInit()) return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 8); // 4x MSAA (puedes probar 2,4,8)

    GLFWwindow* window = glfwCreateWindow(1920, 1080, "Arcane 3D Viewer", nullptr, nullptr);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);

    glfwSwapInterval(1);    //Vsync limit
    
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) { std::cerr<<"Failed to init GLAD\n"; return -1; }
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE); // Habilitar MSAA

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // --- Variable de input ---
    int Numero_Nodos_Total = 36;  
    int Numero_Nodos_Inicial = 2;

    int id_nodo_1 = 0;  
    int id_nodo_2 = 1;

    // --- Crear arcano ---
    std::unique_ptr<Arcane> arcane = std::make_unique<Arcane>(Numero_Nodos_Total, Numero_Nodos_Inicial);

    // --- Esferas ---
    std::vector<glm::vec3> posiciones = arcane->getNodePositions();
    std::vector<glm::vec3> colores = arcane->getNodeColors();
    std::vector<float> sphereVertices = getSphereVertices();
    std::vector<unsigned int> sphereIndices = getSphereIndices();
    size_t sphereInstanceCount = posiciones.size();
    // --- Lineas/Flechas ---
    std::vector<glm::mat4> arrowposiciones = arcane->getArrowTransforms();
    std::vector<glm::vec3> arrowColors = arcane->getArrowColors();
    std::vector<float> arrowVertices = getArrowVertices();
    std::vector<unsigned int> arrowIndices = getArrowIndices();
    size_t arrowInstanceCount  = arrowposiciones.size();

    // --- Buffers esferas (instanced) ---
    GLuint sphereVAO, sphereVBO, sphereEBO;
    glGenVertexArrays(1, &sphereVAO);
    glGenBuffers(1, &sphereVBO);
    glGenBuffers(1, &sphereEBO);

    glBindVertexArray(sphereVAO);

    glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
    glBufferData(GL_ARRAY_BUFFER, sphereVertices.size() * sizeof(float), sphereVertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphereIndices.size() * sizeof(unsigned int), sphereIndices.data(), GL_STATIC_DRAW);

    // sphere mesh only has positions (x,y,z)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Instancias: posición y color (locations 2 and 3 in shader)
    GLuint instanceVBO[2];
    glGenBuffers(2, instanceVBO);

    // posiciones
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO[0]);
    glBufferData(GL_ARRAY_BUFFER, posiciones.size() * sizeof(glm::vec3), posiciones.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(2);
    glVertexAttribDivisor(2, 1);

    // colores
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO[1]);
    glBufferData(GL_ARRAY_BUFFER, colores.size() * sizeof(glm::vec3), colores.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(3);
    glVertexAttribDivisor(3, 1);

    glBindVertexArray(0);

    // --- Buffers flechas (instanced) ---
    GLuint arrowVAO, arrowVBO, arrowEBO;
    glGenVertexArrays(1, &arrowVAO);
    glGenBuffers(1, &arrowVBO);
    glGenBuffers(1, &arrowEBO);

    glBindVertexArray(arrowVAO);

    glBindBuffer(GL_ARRAY_BUFFER, arrowVBO);
    glBufferData(GL_ARRAY_BUFFER, arrowVertices.size() * sizeof(float), arrowVertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, arrowEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, arrowIndices.size() * sizeof(unsigned int), arrowIndices.data(), GL_STATIC_DRAW);

    // Posición de vértices (arrow mesh only has positions x,y,z)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // --- Buffers de instancias para flechas ---
    GLuint arrowInstanceVBO[2];
    glGenBuffers(2, arrowInstanceVBO);

    // Matrices de transformación (bind to arrow VAO)
    glBindBuffer(GL_ARRAY_BUFFER, arrowInstanceVBO[0]);
    glBufferData(GL_ARRAY_BUFFER, arrowposiciones.size() * sizeof(glm::mat4), arrowposiciones.data(), GL_STATIC_DRAW);

    // set attribute locations 4..7 for mat4
    glBindVertexArray(arrowVAO);
    std::size_t vec4Size = sizeof(glm::vec4);
    for (int j = 0; j < 4; ++j) {
        GLuint loc = 4 + j;
        glEnableVertexAttribArray(loc);
        glVertexAttribPointer(loc, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4) * j));
        glVertexAttribDivisor(loc, 1);
    }

    // Colores por instancia (location 3)
    glBindBuffer(GL_ARRAY_BUFFER, arrowInstanceVBO[1]);
    glBufferData(GL_ARRAY_BUFFER, arrowColors.size() * sizeof(glm::vec3), arrowColors.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glVertexAttribDivisor(3, 1);

    glBindVertexArray(0);

    // --- Crear programas ---
    GLuint arrowShaderProgram  = createProgram(arrowVert, arrowFrag);
    GLuint sphereShaderProgram = createProgram(sphereVert, sphereFrag);

    GLint viewLocArrow = glGetUniformLocation(arrowShaderProgram, "view");
    GLint projLocArrow = glGetUniformLocation(arrowShaderProgram, "projection");
    GLint mvpLocSphere = glGetUniformLocation(sphereShaderProgram, "mvp");

    // --- Loop principal ---
    while (!glfwWindowShouldClose(window)) {
        processInput(window);
        glfwPollEvents();

        glClearColor(0.05f, 0.05f, 0.1f, 1.0f);
        //glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Cámara orbital
        float camX = radius * cosf(glm::radians(yaw)) * cosf(glm::radians(pitch));
        float camY = radius * sinf(glm::radians(pitch));
        float camZ = radius * sinf(glm::radians(yaw)) * cosf(glm::radians(pitch));
        glm::vec3 cameraPos = glm::vec3(camX, camY, camZ);
        glm::mat4 view = glm::lookAt(cameraPos, glm::vec3(0.0f), glm::vec3(0, 1, 0));
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);


        // Dibujar flechas (instanced)
        glUseProgram(arrowShaderProgram);
        glUniformMatrix4fv(viewLocArrow, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLocArrow, 1, GL_FALSE, glm::value_ptr(projection));
        glBindVertexArray(arrowVAO);
        glDrawElementsInstanced(GL_TRIANGLES,
                                static_cast<GLsizei>(arrowIndices.size()),
                                GL_UNSIGNED_INT,
                                0,
                                static_cast<GLsizei>(arrowposiciones.size()));
        glBindVertexArray(0);

        // Dibujar esferas (instanced)
        glUseProgram(sphereShaderProgram);
        glm::mat4 mvp = projection * view;
        glUniformMatrix4fv(mvpLocSphere, 1, GL_FALSE, glm::value_ptr(mvp));
        glBindVertexArray(sphereVAO);
        glDrawElementsInstanced(GL_TRIANGLES,
                                static_cast<GLsizei>(sphereIndices.size()),
                                GL_UNSIGNED_INT,
                                0,
                                static_cast<GLsizei>(posiciones.size()));
        glBindVertexArray(0);

        // --- ImGui ---
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiCond_Once); // posición inicial
        ImGui::SetNextWindowSize(ImVec2(350, 100), ImGuiCond_Once); // tamaño opcional
        ImGui::Begin("Control de Nodos", nullptr,
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

        ImGui::PushItemWidth(100); // ancho en píxeles
        if (ImGui::InputInt("Número de nodos", &Numero_Nodos_Total)) {
            if (Numero_Nodos_Total < 36) Numero_Nodos_Total = 36;
        }
        ImGui::PopItemWidth();

        ImGui::PushItemWidth(100); // ancho en píxeles
        if (ImGui::InputInt("Numero minimo del primer nivel", &Numero_Nodos_Inicial)) {
            if (Numero_Nodos_Inicial < 2) Numero_Nodos_Inicial = 2;
            if (Numero_Nodos_Inicial>Numero_Nodos_Total) Numero_Nodos_Inicial = Numero_Nodos_Total;
        }
        ImGui::PopItemWidth();

        if (ImGui::Button("Generar red")) {
             // --- Regenerar Arcane: destruir y recrear ---
            arcane.reset(new Arcane(Numero_Nodos_Total, Numero_Nodos_Inicial));

            // obtener nuevos datos
            posiciones = arcane->getNodePositions();
            colores    = arcane->getNodeColors();
            arrowposiciones = arcane->getArrowTransforms();
            arrowColors = arcane->getArrowColors();

            sphereInstanceCount = posiciones.size();
            arrowInstanceCount  = arrowposiciones.size();

            // --- Re-buffer instancias de esferas ---
            glBindBuffer(GL_ARRAY_BUFFER, instanceVBO[0]);
            glBufferData(GL_ARRAY_BUFFER, sphereInstanceCount * sizeof(glm::vec3), posiciones.data(), GL_DYNAMIC_DRAW);

            glBindBuffer(GL_ARRAY_BUFFER, instanceVBO[1]);
            glBufferData(GL_ARRAY_BUFFER, sphereInstanceCount * sizeof(glm::vec3), colores.data(), GL_DYNAMIC_DRAW);

            // --- Re-buffer instancias de flechas ---
            glBindBuffer(GL_ARRAY_BUFFER, arrowInstanceVBO[0]);
            glBufferData(GL_ARRAY_BUFFER, arrowInstanceCount * sizeof(glm::mat4), arrowposiciones.data(), GL_DYNAMIC_DRAW);

            glBindBuffer(GL_ARRAY_BUFFER, arrowInstanceVBO[1]);
            glBufferData(GL_ARRAY_BUFFER, arrowInstanceCount * sizeof(glm::vec3), arrowColors.data(), GL_DYNAMIC_DRAW);

            // limpiar binding
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }

        ImGui::End();

        ImGui::SetNextWindowPos(ImVec2(20, 120), ImGuiCond_Once);
        ImGui::SetNextWindowSize(ImVec2(200, 100), ImGuiCond_Once);
        ImGui::Begin("Algoritmo de busqueda", nullptr,
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

        ImGui::PushItemWidth(100); // ancho en píxeles
        if (ImGui::InputInt("ID_Nodo #1", &id_nodo_1)) {
            if (id_nodo_1 > Numero_Nodos_Total-1) id_nodo_1 = Numero_Nodos_Total-1;
            if (id_nodo_1 < 0) id_nodo_1 = 0;
        }
        ImGui::PopItemWidth();

        ImGui::PushItemWidth(100); // ancho en píxeles
        if (ImGui::InputInt("ID_Nodo #2", &id_nodo_2)) {
            if (id_nodo_2 > Numero_Nodos_Total-1) id_nodo_2 = Numero_Nodos_Total-1;
            if (id_nodo_2 < 0) id_nodo_2 = 0;
        }
        ImGui::PopItemWidth();

        if (ImGui::Button("Buscar Nodo")) {
            // --- Actualizar flechas si hay ruta ---
            arcane->AsignarColorFlechas();
            colores = arcane->getNodeColors();
            
            std::vector<const Node*> path = arcane->ruta(id_nodo_1,id_nodo_2);
            arrowColors = arcane->ruta_flechas(path, glm::vec3(1.0f,1.0f,1.0f));

            glBindBuffer(GL_ARRAY_BUFFER, arrowInstanceVBO[1]);
            glBufferSubData(GL_ARRAY_BUFFER, 0, arrowColors.size() * sizeof(glm::vec3), arrowColors.data());
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            // --- Colorear los nodos inicial y final ---
            if (!path.empty()) {
                int id_inicio = path.front()->id;
                int id_final  = path.back()->id;

                colores[id_inicio] = glm::vec3(1.0f,1.0f,1.0f);
                colores[id_final]  = glm::vec3(1.0f,1.0f,1.0f);

                glBindBuffer(GL_ARRAY_BUFFER, instanceVBO[1]);
                glBufferSubData(GL_ARRAY_BUFFER, 0, colores.size() * sizeof(glm::vec3), colores.data());
                glBindBuffer(GL_ARRAY_BUFFER, 0);
            }

        }

        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // --- Cleanup ---
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
    return 0;
}