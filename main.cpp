#include <iostream>
#include <vector>
#include <iomanip>  // Add this for table formatting
#include "Body.h"
#include "Octree.h"
#include "utils.h"
#include "Simulation.h"
#include <ctime>
#include <chrono>
#include <thread>
#include <fstream>

void Run(Simulation& sim, int step, int type) {
    const std::clock_t c_start = std::clock();
    auto t_start = std::chrono::high_resolution_clock::now();
    int nframes = 100;

    for (int frame = 0; frame < nframes; ++frame) {
        if (type == 0) sim.step();
        else sim.Brute_step();
    }
    const std::clock_t c_end = std::clock();
    const auto t_end = std::chrono::high_resolution_clock::now();

    float time = 1000.0f * (c_end - c_start) / CLOCKS_PER_SEC;
    float fps = 1000.0f * nframes / time;

    // Table row formatting
    std::cout << std::left << std::setw(15) << (type == 0 ? "Barnes-Hut" : "Brute-Force")
        << std::setw(10) << step
        << std::fixed << std::setprecision(2)
        << std::setw(15) << time
        << std::setw(10) << fps
        << std::endl;
}

void Simula_data() {

    // Print table header
    std::cout << std::left
        << std::setw(15) << "Method"
        << std::setw(10) << "Bodies"
        << std::setw(15) << "Time (ms)"
        << std::setw(10) << "FPS"
        << "\n-------------------------------------------------\n";

    int sim_step[] = { 1000,2000,5000,10000, 25000, 50000, 100000, 500000 , 1000000, 2000000 };

    for (int x : sim_step) {
        Simulation sim(x);
        Run(sim, x, 0);  // Barnes-Hut
        Run(sim, x, 1);  // Brute-Force
    }

}


int main() {
    try {
        Simula_data();
    }
    catch (const std::exception& e) {
        std::cerr << "Simulation error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}