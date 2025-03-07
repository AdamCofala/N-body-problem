#pragma once

#include <glm/glm.hpp>

class Body {
public:

    glm::vec3 pos;
    glm::vec3 vel;
    glm::vec3 acc;
    float mass;

    Body() = default;
    Body(glm::vec3 pos = { 0.0f,0.0f,0.0f }, glm::vec3 vel={0.0f,0.0f,0.0f}, float mass = 0.0f) {
        this->pos;
        this->vel;
        this->mass;
        acc = { 0.0f,0.0f,0.0f };
    }

    void update(float dt) {
        pos += vel * dt;
        vel += acc * dt;
        acc *= 0.0f;
    }
};