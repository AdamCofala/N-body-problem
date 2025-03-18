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

    // Add central massive body (center of everything)
    const float inner_radius = 100.0f;
    const float central_mass = 1e6f;
    bodies.emplace_back(
        glm::vec3(0.0f, 0.0f, 0.0f),  // Position (0,0,0)
        glm::vec3(0.0f),  // Velocity (0,0,0)
        central_mass,
        inner_radius
    );


    // Random number generators
    std::mt19937 gen(0);
    std::uniform_real_distribution<float> angle_dist(0.0f, 2.0f * M_PI);
    std::uniform_real_distribution<float> radius_dist(0.0f, 1.0f);

    // Random mass distribution
    std::uniform_real_distribution<float> mass_dist(1.0f, 50.0f);

    // Galaxy disc parameters
    const float outer_radius = float(std::sqrt(n)) * 5.0f;
    const float t = inner_radius / outer_radius;
    const float vertical_scale = outer_radius * 0.1f; // Disc thickness (10% of radius)
    std::normal_distribution<float> z_dist(0.0f, vertical_scale);

    // Generate all other bodies with random masses
    for (size_t i = 1; i < n; i++) {

        const float angle = angle_dist(gen);
        const float r = radius_dist(gen) * (1.0f - t * t) + t * t;

        // Radial coordinates in disc plane
        const glm::vec2 radial_dir(std::cos(angle), std::sin(angle));
        const float radial_distance = outer_radius * std::sqrt(r);

        // 3D position with Gaussian distribution on Z axis
        const glm::vec3 pos(
            radial_dir.x * radial_distance,
            radial_dir.y * radial_distance,
            z_dist(gen)
        );

        // Generate random mass
        float body_mass = mass_dist(gen);

        // Radius based on mass
        float body_radius = std::cbrt(body_mass);

        // Tangential velocity (only in disc plane)
        const glm::vec3 vel_dir = glm::vec3(radial_dir.y, -radial_dir.x, 0.0f);

        bodies.emplace_back(
            pos,
            vel_dir,  // Just direction for now, magnitude will be set later
            body_mass,
            body_radius
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