// c++ libraries
#include <iostream>
#include <vector>
#include <iomanip>
#include <string>
#include <ctime>
#include <chrono>
#include <thread>
#include <fstream>

//Classes 
#include "Body.h"
#include "Octree.h"
#include "utils.h"
#include "Simulation.h"
#include "Camera.h"

//rendering things
#define GLFW_INCLUDE_NONE
#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Global variables
GLuint  SCR_WIDTH = 1200;
GLuint  SCR_HEIGHT = 720;
GLuint  oldSCR_WIDTH = 1200;
GLuint  oldSCR_HEIGHT = 720;
GLuint  mouseX;
GLuint  mouseY;
bool    firstMouse=true;
float   lastX = SCR_WIDTH / 2.0f;
float   lastY = SCR_HEIGHT / 2.0f;
float   aspectRatio;

//Simulation settings
const int N = 20000;
float     scale = 10.0f;

//Time & camera variables
int       frame = 0;
float     FPS = 0;
float deltaTime = 0.0f;
float lastFrame = 0.0f;
float FOV       = 80.0f;

//Rendering objects
GLFWwindow* window      = nullptr;
std::string windowTitle = "N-body probmem, FPS: ";
//buffers
GLuint      VAO, VBO,   shaderProgram;
GLfloat*    vertices    = new GLfloat[size_t(N) * 3];
GLuint      colorVBO;
GLfloat*    colors;

//Created objects
Camera     camera(glm::vec3(0.0f, 0.0f, 0.0f));
Simulation sim(N);


// Shader sources
const char* vertexShaderSource = R"(#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;  // New color attribute
out vec3 Color;                        // Pass to fragment shader

uniform mat4 view;
uniform mat4 projection;

void main() {
    gl_Position = projection * view * vec4(aPos, 1.0);
    gl_PointSize = 3.0;
    Color = aColor;  // Pass color to fragment shader
})";

const char* fragmentShaderSource = R"(#version 330 core
in vec3 Color;  // Color from vertex shader
out vec4 FragColor;

void main() {
    vec2 center = gl_PointCoord - vec2(0.5);
    float dist = length(center);
    float alpha = 1.0 - smoothstep(0.48, 0.5, dist);
    
    FragColor = vec4(Color, alpha);  // Use dynamic color
})";

//3D stuff
    void setMat3(const std::string & name, const glm::mat3 & mat)
    {
        glUniformMatrix3fv(glGetUniformLocation(shaderProgram, name.c_str()), 1, GL_FALSE, &mat[0][0]);

    }
    void setMat4(const std::string & name, const glm::mat4 & mat)
    {
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }
    void use()
    {
        glUseProgram(shaderProgram);
    }
    void setVec3(const std::string & name, float x, float y, float z)
    {
        glUniform3f(glGetUniformLocation(shaderProgram, name.c_str()), x, y, z);
    }

// Function prototypes
bool initializeGLFW();
bool createShaders();
void setupBuffers();
void update();
void draw();
void cleanup();
void addToBuffer();
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

// Initialization functions
bool initializeGLFW() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(SCR_WIDTH,SCR_HEIGHT, "N-Body Simulation", NULL, NULL);
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

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
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

    //3D camera
    if (!(SCR_WIDTH == 0 || SCR_HEIGHT == 0))
        aspectRatio = (float)SCR_WIDTH / (float)SCR_HEIGHT;

    glm::mat4 projection = glm::perspective(glm::radians(camera.horizontalFov), aspectRatio, 0.1f, 10000.0f);
    glm::mat4 view = camera.GetViewMatrix();
    camera.updateRadius(deltaTime);

    use();
    setVec3("viewPos", camera.Position.x, camera.Position.y, camera.Position.z);
    setMat4("projection", projection);
    setMat4("view", view);

    return true;
}

void setupBuffers() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &colorVBO);  // Create color VBO

    glBindVertexArray(VAO);

    // Position buffer setup
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, N * 3 * sizeof(GLfloat), nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Color buffer setup
    glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
    glBufferData(GL_ARRAY_BUFFER, N * 3 * sizeof(GLfloat), nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Initialize color array
    colors = new GLfloat[N * 3];

    // Configure OpenGL (keep your existing blend settings if needed)
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);  // Changed for better blending
    glEnable(GL_PROGRAM_POINT_SIZE);
}

// Runtime functions
void update() {
    sim.dt = deltaTime;
    sim.step();
    addToBuffer();
}

float horizontalToVerticalFOV(float horizontalFOV, float aspectRatio) {
    return 2.0f * atan(tan(glm::radians(horizontalFOV) / 2.0f) / aspectRatio);
}

void draw() {

    float currentFrame = static_cast<float>(glfwGetTime());
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
    frame++;
    if (frame % 5 == 0) {
        FPS = (1 / deltaTime);
        glfwSetWindowTitle(window, (windowTitle + std::to_string(FPS)).c_str());
    }
    

    processInput(window);
    
    if (lastX != mouseX || lastY != mouseX || oldSCR_WIDTH != SCR_WIDTH || oldSCR_HEIGHT != SCR_HEIGHT ) {
   
        if (!(SCR_WIDTH == 0 || SCR_HEIGHT == 0))
            aspectRatio = (float)SCR_WIDTH / (float)SCR_HEIGHT;
    
        glm::mat4 projection = glm::perspective(glm::radians(camera.horizontalFov), aspectRatio, 0.1f, 10000.0f);
        glm::mat4 view = camera.GetViewMatrix();
        camera.updateRadius(deltaTime);
    
        use();
        setVec3("viewPos", camera.Position.x, camera.Position.y, camera.Position.z);
        setMat4("projection", projection);
        setMat4("view", view);
    }
    
    oldSCR_HEIGHT = SCR_HEIGHT;
    oldSCR_WIDTH = SCR_WIDTH;

    glClearColor(0.0f, 0.0f, 0.01f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(shaderProgram);
    glBindVertexArray(VAO);
    glDrawArrays(GL_POINTS, 0, N);

    glfwSwapBuffers(window);
}

void addToBuffer() {
#pragma omp parallel for
    for (int i = 0; i < N; i++) {
        // Position calculation (existing)
        vertices[i * 3] = sim.bodies[i].pos.y ;
        vertices[i * 3 + 1] = sim.bodies[i].pos.z ;
        vertices[i * 3 + 2] = sim.bodies[i].pos.x ;

        float max_speed = 500.0f;
        // New: Color calculation based on velocity (example)
        float speed = sqrt(sim.bodies[i].pos.x * sim.bodies[i].pos.x + sim.bodies[i].pos.y * sim.bodies[i].pos.y);
        float normalized_speed = (speed / max_speed);
        normalized_speed = std::min(1.0f, std::max(0.0f, normalized_speed));

        colors[i * 3] = (1.0f - normalized_speed);       // Red component
        colors[i * 3 + 1] = 0.2f;     // Green component
        colors[i * 3 + 2] = normalized_speed; ;  // Blue component
    }

    // Update position buffer
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, N * 3 * sizeof(GLfloat), vertices);

    // Update color buffer
    glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, N * 3 * sizeof(GLfloat), colors);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void cleanup() {
    delete[] colors;
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

//Calbacks
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{

    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    SCR_HEIGHT = height;
    SCR_WIDTH = width;

    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window)
{

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }

    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        camera.ProcessKeyboard(UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
       camera.ProcessKeyboard(DOWN, deltaTime);

}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    mouseX = xpos;
    mouseY = ypos;

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;


    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}


