#include <iostream>
#include <vector>
#include "Body.h"
#include "Octree.h"
#include "utils.h"
#include "Simulation.h"
#include <ctime>
#include <chrono>
#include <thread>

int main() {
    try {
        
        int sim_step[] = { 1000,2000,5000,10000, 25000, 50000, 100000, 500000 , 1000000, 2000000 };

        for (int x : sim_step) {

            Simulation sim(x);

            // Run simulation for 1000 frames
            const std::clock_t c_start = std::clock();
            auto t_start = std::chrono::high_resolution_clock::now();
            int nframes = 1000;
            for (int frame = 0; frame < nframes; ++frame) {
                sim.step();
              
              //// Basic status output
              //std::cout << "Frame " << frame
              //    << " | Position: (" << sim.bodies[0].pos.x
              //    << ", " << sim.bodies[0].pos.y
              //    << ", " << sim.bodies[0].pos.z << ")\n";
            }

            const std::clock_t c_end = std::clock();
            const auto t_end = std::chrono::high_resolution_clock::now();


            float time = 1000.0f * (c_end - c_start) / CLOCKS_PER_SEC;
            std::cout << "For " << x << " : " << std::endl;
            std::cout << std::fixed << "CPU time used:  " << time << "ms\n" << "Which is: " << 1000.0f * nframes / time << "fps" << std::endl;
        
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Simulation error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}