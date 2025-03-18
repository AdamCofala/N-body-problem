/*#pragma once
#pragma once
#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

bool cursorSwap = 0;


// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

// Default camera values
#define YAW  -90.0f
#define PITCH  0.0f
#define SPEED  2.5f
#define SENSITIVITY 0.08f
#define ZOOM  70.0f

//extern bool cursorSwap;



// An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
class Camera
{
public:
    // camera Attributes
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;
    // euler Angles
    float Yaw;
    float Pitch;
    // camera options
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;

    // constructor with vectors
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
    {
        Position = position;
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }
    // constructor with scalar values
    Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
    {
        Position = glm::vec3(posX, posY, posZ);
        WorldUp = glm::vec3(upX, upY, upZ);
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }

    // returns the view matrix calculated using Euler Angles and the LookAt Matrix
    glm::mat4 GetViewMatrix()
    {
        return glm::lookAt(Position, Position + Front, Up);
        //return glm::lookAt(0, 0, 0);
    }

    // processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
    void ProcessKeyboard(Camera_Movement direction, float deltaTime)
    {

        float velocity = MovementSpeed * deltaTime;
        if (direction == FORWARD)
            Position += Front * velocity;
        if (direction == BACKWARD)
            Position -= Front * velocity;
        if (direction == LEFT)
            Position -= Right * velocity;
        if (direction == RIGHT)
            Position += Right * velocity;
        if (direction == UP)
            Position += Up * velocity;
        if (direction == DOWN)
            Position += -Up * velocity;



    }

    // processes input received from a mouse input system. Expects the offset value in both the x and y direction.
    void ProcessMouseMovement(float xoffset, float yoffset, float deltaTime)
    {
        GLboolean constrainPitch = true;

        static float lastPitch = Pitch;
        static float lastYaw = Yaw;

        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        Yaw += xoffset;
        Pitch += yoffset;
        //Position += Right * velocity;
        float velocity = MovementSpeed * deltaTime;
        Position += Right * velocity * -(Yaw-lastYaw) * 3.0f;
        Position += Up * velocity * -(Pitch-lastPitch) * 3.0f;


        lastPitch = Pitch;
        lastYaw = Yaw;
        // make sure that when pitch is out of bounds, screen doesn't get flipped
        if (constrainPitch)
        {
            if (Pitch > 89.0f)
                Pitch = 89.0f;
            if (Pitch < -89.0f)
                Pitch = -89.0f;
        }

        // update Front, Right and Up Vectors using the updated Euler angles
        updateCameraVectors();
    }

    // processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
    void ProcessMouseScroll(float yoffset)
    {
        //Zoom -= (float)yoffset;
        MovementSpeed -= (float)yoffset * 2;
        if (MovementSpeed < 1.0f)
            MovementSpeed = 1.0f;
        if (MovementSpeed > 50.0f)
            MovementSpeed = 50.0f;
    }

private:
    // calculates the front vector from the Camera's (updated) Euler Angles
    void updateCameraVectors()
    {
        // calculate the new Front vector
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);
        // also re-calculate the Right and Up vector
        Right = glm::normalize(glm::cross(Front, WorldUp));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
        Up = glm::normalize(glm::cross(Right, Front));
    }
};

extern Camera camera;

#endif*/
#pragma once
#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

// Default camera values
#define YAW  -45.0f
#define PITCH  25.0f
#define SENSITIVITY 0.15f
#define RADIUS 1500.0f // Default distance from the target
#define ZOOM  90.0f
#define MOVEMENTSPEED  400.0f

class Camera {
public:

    // Camera attributes
    glm::vec3 Position;
    glm::vec3 Target; // The point to orbit around
    glm::vec3 Up;

    // Euler angles
    float Yaw;
    float Pitch;

    // Camera options
    float movementSpeed = MOVEMENTSPEED;
    float MouseSensitivity = SENSITIVITY;
    float Radius;
    float desiredRadius;
    float zoom = ZOOM;
    float horizontalFov = 70;

    // Constructor
    Camera(glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f), float radius = RADIUS)
        : Target(target), Radius(radius), desiredRadius(radius), Yaw(YAW), Pitch(PITCH), MouseSensitivity(SENSITIVITY), Up(glm::vec3(0.0f, 1.0f, 0.0f)) {
        updateCameraPosition();
    }

    // Update camera position based on Yaw and Pitch
    void updateCameraPosition()
    {
        float x = Radius * cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        float y = Radius * sin(glm::radians(Pitch));
        float z = Radius * sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Position = Target + glm::vec3(x, y, z);
       // std::cout << Position.x << std::endl << Position.y << std::endl << Position.z << std::endl;
    }

    void updateRadius(float deltaTime)
    {
        if (desiredRadius > Radius)
        {
            Radius = Radius + ((desiredRadius - Radius) * 9 * deltaTime);

            if (Radius > desiredRadius)
                Radius = desiredRadius;
        }
        else if (desiredRadius < Radius)
        {
            Radius = Radius - ((Radius - desiredRadius) * 9 * deltaTime);

            if (Radius < desiredRadius)
                Radius = desiredRadius;
        }
       

        updateCameraPosition();
    }

    // Get the view matrix
    glm::mat4 GetViewMatrix() {
        return glm::lookAt(Position, Target, Up);
    }

    float getHorizontalFov()
    {
        return horizontalFov;
    }

    // Process mouse movement
    void ProcessMouseMovement(float xoffset, float yoffset)
    {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        Yaw += xoffset;
        Pitch -= yoffset;

        // Constrain pitch
        if (Pitch > 89.9f) Pitch = 89.9f;
        if (Pitch < -89.9f) Pitch = -89.9f;

        updateCameraPosition();
    }

    // Process mouse scroll (optional: to change radius)
    void ProcessMouseScroll(float yoffset)
    {
        
        desiredRadius -= (float)yoffset * 10;
        if (desiredRadius < 1.0f) desiredRadius = 1.0f;
        //std::cout << desiredRadius << std::endl;
        updateCameraPosition();
    }

    void ProcessKeyboard(Camera_Movement direction, float deltaTime)
    {
        float velocity = movementSpeed * deltaTime;
        if (direction == UP)
            Target[1] += velocity;
        if (direction == DOWN)
            Target[1] += -velocity;

        updateCameraPosition();
    }
};

#endif // CAMERA_H
