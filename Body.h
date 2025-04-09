#pragma once

#include <algorithm>
#include <glm/glm.hpp>


class Body {
public:

    glm::vec3 pos;
    glm::vec3 vel;
    glm::vec3 acc;
    float mass;
    float radius;

    Body(glm::vec3 pos = { 0.0f, 0.0f, 0.0f }, glm::vec3 vel = { 0.0f, 0.0f, 0.0f }, float mass = 1.0f, float radius = 1.0f)
        : pos(pos), vel(vel), mass(mass), radius(radius), acc(0.0f)
    {
        if (mass <= 0.0f) {
            throw std::invalid_argument("Mass must be positive.");
        }
    }

    glm::vec3 getColor(int type, int typeColor) {

        glm::vec3 Color(0.0f, 0.0f, 0.0f);

        float min_speed = 40.0f;  // Minimum speed (blue)
        float max_speed = 0;
        if (type == 0)  max_speed = 90.0f;
        else if (type == 1) max_speed = 120.0f;

        float speed = sqrt(
            vel.x * vel.x +
            vel.y * vel.y +
            vel.z * vel.z
        );

        if (typeColor == 0) {
            float normalized_speed = (speed - min_speed) / (max_speed - min_speed);
            normalized_speed = std::clamp(normalized_speed, 0.0f, 1.0f);

            Color[0] = normalized_speed;
            Color[1] = 0.1f;
            Color[2] = 1.0f - normalized_speed;
        }
        else {
            float normalized_mass = mass/50.0f;

            Color[0] = normalized_mass;
            Color[1] = 0.1f;
            Color[2] = 1.0f - normalized_mass;
        }


        return Color;
    }

    void update(float dt) {
        
        /*idk know if pos should be updated before vel, however someone with more experience than me do like that, so i hope
        its correct ;-; */
        pos += vel * dt; 
        vel += acc * dt;
        acc *= 0.0f;
    }

    

};