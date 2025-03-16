#include <iostream>
#include <vector>
#include <iomanip>
#include "Body.h"
#include "Octree.h"
#include "utils.h"
#include "Simulation.h"
#include <ctime>
#include <chrono>
#include <thread>
#include <fstream>
#define GLFW_INCLUDE_NONE
#include <glad/glad.h> 
#include <GLFW/glfw3.h>

// Global variables
const int width = 1200;
const int height = 720;
const int N = 20000;
float scale = 7.0f;

GLFWwindow* window = nullptr;
GLuint VAO, VBO, shaderProgram;
GLfloat* vertices = new GLfloat[size_t(N) * 3];
Simulation sim(N);

// Shader sources
const char* vertexShaderSource = R"(#version 330 core
layout (location = 0) in vec3 aPos;
void main() {
    gl_Position = vec4(aPos, 1.0);
    gl_PointSize = 3.0;
})";

const char* fragmentShaderSource = R"(#version 330 core
out vec4 FragColor;
void main() {
    FragColor = vec4(0.8f, 0.3f, 0.02f, 1.0f);
})";

// Function prototypes
bool initializeGLFW();
bool createShaders();
void setupBuffers();
void update();
void draw();
void cleanup();
void addToBuffer();

// Initialization functions
bool initializeGLFW() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, "N-Body Simulation", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return false;
    }

    glViewport(0, 0, width, height);
    return true;
}

bool createShaders() {
    // Vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    // Fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    // Shader program
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Error checking
    GLint success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "Shader linking failed: " << infoLog << std::endl;
        return false;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return true;
}

void setupBuffers() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // Initialize buffer with empty data
    glBufferData(GL_ARRAY_BUFFER, N * 3 * sizeof(GLfloat), nullptr, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Configure OpenGL
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_PROGRAM_POINT_SIZE);
}

// Runtime functions
void update() {
    sim.step();
    addToBuffer();
}

void draw() {
    glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(shaderProgram);
    glBindVertexArray(VAO);
    glDrawArrays(GL_POINTS, 0, N);

    glfwSwapBuffers(window);
}

void addToBuffer() {
    for (int i = 0; i < N; i++) {
        // Normalize positions to [-1, 1] range
        vertices[i * 3] = sim.bodies[i].pos.x / (100.0f * scale);
        vertices[i * 3 + 1] = sim.bodies[i].pos.y / (100.0f * scale);
        vertices[i * 3 + 2] = sim.bodies[i].pos.z / (100.0f * scale);
    }

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, N * 3 * sizeof(GLfloat), vertices);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

// Cleanup function
void cleanup() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);
    glfwDestroyWindow(window);
    glfwTerminate();
    delete[] vertices;
}

// Main function
int main() {
    if (!initializeGLFW()) return -1;
    if (!createShaders()) return -1;
    setupBuffers();

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        update();
        draw();
    }

    cleanup();
    return 0;
}