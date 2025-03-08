#pragma once

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

    void update(float dt) {
        pos += vel * dt;
        vel += acc * dt;
        acc *= 0.0f;
    }
};