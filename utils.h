#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include <vector>
#include <random>
#include <algorithm>
#include <glm/glm.hpp>
#include "Body.h"
#include <cmath>
#define M_PI 3.14159265358979323846f  /* pi */

std::vector<Body> uniform_disc(size_t n) {
    std::vector<Body> bodies;
    bodies.reserve(n);

    // Add central massive body
    const float inner_radius = 25.0f;
    const float central_mass = 1e6f;
    bodies.emplace_back(
        glm::vec3(0.0f),  // Position (0,0,0)
        glm::vec3(0.0f),  // Velocity (0,0,0)
        central_mass,
        inner_radius
    );

    // Random number setup with fixed seed for reproducibility
    std::mt19937 gen(0);
    std::uniform_real_distribution<float> angle_dist(0.0f, 2.0f*M_PI);
    std::uniform_real_distribution<float> radius_dist(0.0f, 1.0f);

    // Calculate disc parameters
    const float outer_radius = float(std::sqrt(n)) * 5.0f;
    const float t = inner_radius / outer_radius;

    // Generate orbiting bodies
    while (bodies.size() < n) {
        const float angle = angle_dist(gen);
        const float r = radius_dist(gen) * (1.0f - t * t) + t * t;

        // Convert to Cartesian coordinates (x, y, 0)
        const glm::vec3 dir(
            std::cos(angle),
            std::sin(angle),
            0.0f
        );

        const glm::vec3 pos = dir * outer_radius * std::sqrt(r);
        const glm::vec3 vel = glm::vec3(dir.y, -dir.x, 0);  // Tangent direction

        bodies.emplace_back(
            pos,
            vel,
            1.0f,    // Mass
            std::cbrt(1.0f)  // Radius = mass^(1/3)
        );
    }

    // Sort bodies by distance from center (skip central body)
    std::sort(bodies.begin() + 1, bodies.end(),
        [](const Body& a, const Body& b) {
        return glm::dot(a.pos, a.pos) < glm::dot(b.pos, b.pos);
    });

    // Calculate velocities based on enclosed mass
    float total_mass = bodies[0].mass;  // Start with central mass
    for (size_t i = 1; i < bodies.size(); ++i) {
        auto& body = bodies[i];
        const float distance = glm::length(body.pos);

        // Orbital velocity: v = sqrt(G*M_enclosed/r)
        // Using Newtonian gravity with G=1
        const float v = std::sqrt(total_mass / distance);
        body.vel *= v;  // Apply to precomputed tangent direction

        total_mass += body.mass;  // Accumulate mass for next iterations
    }

    return bodies;
}