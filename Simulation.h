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
        octree(0.8f, 0.1f, Octant(bodies))  // Direct initialization
    {
    }

    void step() {
        attract();  // Calculate forces first
        iterate();  // Update positions/velocities
        frame++;
    }

private:
    void iterate() {
        #pragma omp parallel for
        for (int i = 0; i < static_cast<int>(bodies.size()); ++i) {
            bodies[i].vel += bodies[i].acc * dt;
            bodies[i].pos += bodies[i].vel * dt;
            bodies[i].acc = glm::vec3(0.0f);
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
        for (int i = 0; i < static_cast<int>(bodies.size()); ++i) {
            bodies[i].acc = octree.acc(bodies[i].pos);
        }
    }

};
