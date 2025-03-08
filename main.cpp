#include <iostream>
#include <vector>
#include "Body.h"
#include "Octree.h"
#include "utils.h"
#include "Simulation.h"

int main() {
    try {
        Simulation sim;

        // Run simulation for 100 frames
        for (int frame = 0; frame < 1000; ++frame) {
            sim.step();

            // Basic status output
            std::cout << "Frame " << frame
                << " | Position: (" << sim.bodies[0].pos.x
                << ", " << sim.bodies[0].pos.y
                << ", " << sim.bodies[0].pos.z << ")\n";
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Simulation error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}