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

std::vector<Body> uniform_disc_bin(size_t n) {
    std::vector<Body> bodies;
    bodies.reserve(n);

    // Random number generators
    std::mt19937 gen(0);
    std::uniform_real_distribution<float> angle_dist(0.0f, 2.0f * M_PI);
    std::uniform_real_distribution<float> radius_dist(0.0f, 1.0f);
    std::uniform_real_distribution<float> mass_dist(1.0f, 50.0f);

    // Binary system parameters
    const float galaxy_separation = float(std::sqrt(n)) * 8.0f; // Distance between galaxy centers
    const float inner_radius = 100.0f;
    const float central_mass = 1e6f; // Reduced from 1e6f to have two smaller black holes

    // Binary system orbital parameters
    const float binary_angle = angle_dist(gen); // Random angle for binary system orientation
    const glm::vec2 binary_dir(std::cos(binary_angle), std::sin(binary_angle));

    // Create the two central massive bodies (centers of each galaxy)
    // First galaxy center
    const glm::vec3 center1(-binary_dir.x * galaxy_separation / 2, -binary_dir.y * galaxy_separation / 2, 0.0f);
    // Calculate orbital velocity for stable binary system
    const float binary_orbital_velocity = std::sqrt(central_mass / galaxy_separation)/1.2f;

    const glm::vec3 vel1(binary_dir.y * binary_orbital_velocity, -binary_dir.x * binary_orbital_velocity, 0.0f);

    bodies.emplace_back(
        center1,
        vel1,
        central_mass,
        inner_radius
    );

    // Second galaxy center
    const glm::vec3 center2(binary_dir.x * galaxy_separation / 2, binary_dir.y * galaxy_separation / 2, 0.0f);
    const glm::vec3 vel2(-binary_dir.y * binary_orbital_velocity, binary_dir.x * binary_orbital_velocity, 0.0f);

    bodies.emplace_back(
        center2,
        vel2,
        central_mass,
        inner_radius
    );

    // Split remaining particles between two galaxies
    const size_t bodies_per_galaxy = (n - 2) / 2;
    const size_t first_galaxy_bodies = bodies_per_galaxy + ((n - 2) % 2); // First galaxy gets extra particle if odd

    // Galaxy disc parameters
    const float outer_radius = float(std::sqrt(n / 2)) * 5.0f; // Adjusted for half the particles per galaxy
    const float t = inner_radius / outer_radius;
    const float vertical_scale = outer_radius * 0.1f; // Disc thickness (10% of radius)
    std::normal_distribution<float> z_dist(0.0f, vertical_scale);

    // ---- GENERATE FIRST GALAXY ----

    // Add all first galaxy bodies with initial positions and velocity directions
    std::vector<size_t> first_galaxy_indices;
    first_galaxy_indices.reserve(first_galaxy_bodies);

    for (size_t i = 0; i < first_galaxy_bodies; i++) {
        const float angle = angle_dist(gen);
        const float r = radius_dist(gen) * (1.0f - t * t) + t * t;

        // Radial coordinates in disc plane
        const glm::vec2 radial_dir(std::cos(angle), std::sin(angle));
        const float radial_distance = outer_radius * std::sqrt(r);

        // 3D position with Gaussian distribution on Z axis, relative to galaxy center
        const glm::vec3 pos = center1 + glm::vec3(
            radial_dir.x * radial_distance,
            radial_dir.y * radial_distance,
            z_dist(gen)
        );

        // Generate random mass
        float body_mass = mass_dist(gen);

        // Radius based on mass
        float body_radius = std::cbrt(body_mass);

        // Tangential velocity direction within the galaxy disc
        // This is perpendicular to the radial direction in the disc plane
        const glm::vec3 internal_vel_dir = glm::vec3(radial_dir.y, -radial_dir.x, 0.0f);

        first_galaxy_indices.push_back(bodies.size());

        bodies.emplace_back(
            pos,
            internal_vel_dir, // Store only direction for now, will set magnitude later
            body_mass,
            body_radius
        );
    }

    // Calculate velocities for first galaxy based on enclosed mass
    // First, sort indices by distance from galaxy center
    std::sort(first_galaxy_indices.begin(), first_galaxy_indices.end(),
        [&bodies, center1](size_t a, size_t b) {
        return glm::length(bodies[a].pos - center1) < glm::length(bodies[b].pos - center1);
    });

    // Adjust orbital velocities around first galaxy center
    float total_mass1 = bodies[0].mass; // Start with central mass
    for (size_t idx : first_galaxy_indices) {
        auto& body = bodies[idx];

        // Calculate distance from galaxy center
        const glm::vec3 rel_pos = body.pos - center1;
        const float distance = glm::length(rel_pos);

        // Calculate orbital velocity based on enclosed mass
        const float v = std::sqrt(total_mass1 / distance);

        // Apply orbital velocity to the direction vector and add galaxy's own velocity
        body.vel = vel1 + body.vel * v;

        total_mass1 += body.mass; // Accumulate mass for next iterations
    }

    // ---- GENERATE SECOND GALAXY ----

    // Add all second galaxy bodies with initial positions and velocity directions
    std::vector<size_t> second_galaxy_indices;
    second_galaxy_indices.reserve(bodies_per_galaxy);

    for (size_t i = 0; i < bodies_per_galaxy; i++) {
        const float angle = angle_dist(gen);
        const float r = radius_dist(gen) * (1.0f - t * t) + t * t;

        // Radial coordinates in disc plane
        const glm::vec2 radial_dir(std::cos(angle), std::sin(angle));
        const float radial_distance = outer_radius * std::sqrt(r);

        // 3D position with Gaussian distribution on Z axis, relative to second galaxy center
        const glm::vec3 pos = center2 + glm::vec3(
            radial_dir.x * radial_distance,
            radial_dir.y * radial_distance,
            z_dist(gen)
        );

        // Generate random mass
        float body_mass = mass_dist(gen);

        // Radius based on mass
        float body_radius = std::cbrt(body_mass);

        // Tangential velocity direction within the galaxy disc
        // This is perpendicular to the radial direction in the disc plane
        const glm::vec3 internal_vel_dir = glm::vec3(radial_dir.y, -radial_dir.x, 0.0f);

        second_galaxy_indices.push_back(bodies.size());

        bodies.emplace_back(
            pos,
            internal_vel_dir, // Store only direction for now, will set magnitude later
            body_mass,
            body_radius
        );
    }

    // Calculate velocities for second galaxy based on enclosed mass
    // First, sort indices by distance from galaxy center
    std::sort(second_galaxy_indices.begin(), second_galaxy_indices.end(),
        [&bodies, center2](size_t a, size_t b) {
        return glm::length(bodies[a].pos - center2) < glm::length(bodies[b].pos - center2);
    });

    // Adjust orbital velocities around second galaxy center
    float total_mass2 = bodies[1].mass; // Start with second central mass
    for (size_t idx : second_galaxy_indices) {
        auto& body = bodies[idx];

        // Calculate distance from galaxy center
        const glm::vec3 rel_pos = body.pos - center2;
        const float distance = glm::length(rel_pos);

        // Calculate orbital velocity based on enclosed mass
        const float v = std::sqrt(total_mass2 / distance);

        // Apply orbital velocity to the direction vector and add galaxy's own velocity
        body.vel = vel2 + body.vel * v;

        total_mass2 += body.mass; // Accumulate mass for next iterations
    }

    // Ensure we have exactly n bodies
    assert(bodies.size() == n);

    return bodies;
}
