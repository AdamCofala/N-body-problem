#pragma once

#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include "Body.h"
#include "Octree.h"
#include "utils.h"


class Simulation {
public:
    float dt;
    size_t frame;
    std::vector<Body> bodies;
    Octree octree;

    Simulation(int n)
        : dt(0.05f),
        frame(0),
        bodies(uniform_disc(n)),
        octree(0.5f, 0.1f, Octant(bodies))  // Direct initialization
    {
    }

    void step() {
        attract();  // Calculate forces first
        iterate();  // Update positions/velocities
        //collide();  // Handle collisions
        frame++;
    }

private:
    void iterate() {
        for (auto& body : bodies) {
            // Semi-implicit Euler integration
            body.vel += body.acc * dt;
            body.pos += body.vel * dt;
            body.acc = glm::vec3(0.0f);  // Reset acceleration
        }
    }

    void attract() {
        // Rebuild octree with current positions
        Octant new_octant(bodies);
        octree.clear(new_octant);

        for (const auto& body : bodies) {
            octree.insert(body.pos, body.mass);
        }
        octree.propagate();

        // Update accelerations using octree
#pragma omp parallel for
        for (size_t i = 0; i < bodies.size(); ++i) {
            bodies[i].acc = octree.acc(bodies[i].pos);
        }
    }

    void collide() {
        // Spatial partitioning using octree
        std::vector<std::pair<size_t, size_t>> pairs;

        // Build collision pairs using octree
        auto& nodes = octree.nodes;
        std::vector<size_t> stack;
        stack.push_back(0);

        while (!stack.empty()) {
            size_t node_idx = stack.back();
            stack.pop_back();

            if (nodes[node_idx].is_leaf()) {
                // Check against other leaves
                size_t other = nodes[node_idx].next;
                while (other != 0) {
                    if (nodes[node_idx].octant.size + nodes[other].octant.size >
                        glm::length(nodes[node_idx].pos - nodes[other].pos)) {
                        pairs.emplace_back(node_idx, other);
                    }
                    other = nodes[other].next;
                }
            }
            else {
                // Add children to stack
                for (size_t i = 0; i < 8; ++i) {
                    stack.push_back(nodes[node_idx].children + i);
                }
            }
        }

        // Resolve collisions
        for (auto& pair : pairs) {
            resolve(pair.first, pair.second);
        }
    }

    void resolve(size_t i, size_t j) {
        Body& a = bodies[i];
        Body& b = bodies[j];
        const glm::vec3 delta = b.pos - a.pos;
        const float dist = glm::length(delta);
        const float min_dist = a.radius + b.radius;

        if (dist < min_dist && dist > 0.0f) {
            const glm::vec3 normal = delta / dist;
            const glm::vec3 relative_vel = b.vel - a.vel;
            const float impulse = glm::dot(relative_vel, normal);

            // Only resolve if moving towards each other
            if (impulse > 0) return;

            const float restitution = 0.8f;
            const float j = -(1 + restitution) * impulse / (1 / a.mass + 1 / b.mass);

            a.vel -= j / a.mass * normal;
            b.vel += j / b.mass * normal;

            // Position correction
            const float penetration = min_dist - dist;
            const glm::vec3 correction = normal * penetration * 0.2f;
            a.pos -= correction * (b.mass / (a.mass + b.mass));
            b.pos += correction * (a.mass / (a.mass + b.mass));
        }
    }
};
