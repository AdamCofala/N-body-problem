#pragma once

#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include "Body.h"
#include "Octree.h"
#include "utils.h"
constexpr float G = 6.674e-11f;

class Simulation {
public:
    float dt;
    size_t frame;
    std::vector<Body> bodies;
    

    //Simulation Settings:
    float        theta          = 0.8f;
    float        epsilon        = 0.1f;
    unsigned int leaf_capacity  = 4;
    float        cutoffDistance = 1500.0f;

    Octree octree;

    Simulation(int n, int type)
        : dt(0.05f),
        frame(0),
       
        octree(theta, epsilon, Octant(bodies), leaf_capacity, cutoffDistance)  // Direct initialization
    {
        if (type == 0)  bodies = uniform_disc(n);
        else if (type==1) bodies = uniform_disc_bin(n);
        // else if (type==2) bodies =spiral_galaxy(n);
        //...
    }

    void step() {
        attract();  // Calculate forces first
        iterate();  // Update positions/velocities
        frame++;
    }

    void Brute_step() {
        // Phase 1: Compute accelerations
        for (Body& x : bodies) {
            x.acc = glm::vec3(0.0f);
            for (const Body& y : bodies) {
                if (&x == &y) continue; // Skip self-interaction

                // Calculate distance vector
                glm::vec3 r_vec = y.pos - x.pos;
                float r_squared = glm::dot(r_vec, r_vec);

                // Avoid division by zero (add softening length if needed)
                if (r_squared < 1e-10f) continue;

                float r = glm::sqrt(r_squared);
                glm::vec3 direction = r_vec / r; // Normalized direction

                // Newtonian gravity: F = G*(m1*m2)/r², acceleration = F/m1 = G*m2/r²
                x.acc += G * y.mass * direction / r_squared;
            }
        }

        // Phase 2: Update velocities and positions
        for (Body& x : bodies) {
            x.vel += x.acc * dt;
            x.pos += x.vel * dt;
        }

    }

private:
    void iterate() {

        for (int i = 0; i < static_cast<int>(bodies.size()); ++i) {
            bodies[i].vel += bodies[i].acc * dt;
            bodies[i].pos += bodies[i].vel * dt;
            bodies[i].acc = glm::vec3(0.0f);
        }
    }

    void attract() {
        octree.build(bodies);

        // Update accelerations using octree
#pragma omp parallel for
        for (int i = 0; i < static_cast<int>(bodies.size()); ++i) {
            bodies[i].acc = octree.acc(bodies[i].pos,bodies);
        }
    }

};
