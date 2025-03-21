
# N-Body Simulation with OpenGL

A real-time 3D N-body simulation of gravitational interactions, optimized with Barnes-Hut algorithm and rendered using OpenGL. Features interactive camera controls, velocity & mass-based coloring&size, and dynamic parameter adjustments.

## Screenshots

| ![image](https://github.com/user-attachments/assets/ee677e8a-0486-4b43-9d15-909dde6807a8) | ![image](https://github.com/user-attachments/assets/588b5fd1-d867-4e2d-b468-04d2b337d7b0) | ![image](https://github.com/user-attachments/assets/e5a6a5b3-f657-459e-be73-611edd1b6e70) |
|------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| Full Galaxy Overview                                                                                                                                                                     | Top view                                                                                                                                                                | Velocity & mass-based  Color-Size Mapping in details                                                                                                                                                            |


## Features
- **Barnes-Hut Octree Optimization**  
  O(n log n) complexity for efficient force calculations
- **3D Visualization**  
  OpenGL rendering with point sprites and alpha blending
- **Interactive Camera**  
  - Orbital rotation (move mouse)  
  - Zoom (mouse scroll)  
  - Height adjustment (Q/E keys)  
  - Dynamic FOV adjustment
- **Velocity-Based Coloring**  
  Particles colored from blue (fast) to red (slow)
- **Performance**  
  Multi-threaded using OpenMP
- **Galaxy Initialization**  
  Realistic disc-shaped particle distribution

## Dependencies
- OpenGL 4.6
- GLFW 3.3
- GLAD
- GLM 0.9.9
- OpenMP (optional)

## Controls
| Key           | Action                          |
|---------------|---------------------------------|
| `Q/E`         | Move camera target down/up     |
| `R`           | Toggle mouse cursor            |
| `ESC`         | Exit program                   |
| Mouse drag    | Rotate view                    |
| Scroll        | Zoom in/out                    |

## Configuration (Edit in Code)
```cpp
// In main.cpp
const int N = 15000;  // Particle count
float scale = 10.0f;   // Spatial scaling

// In Camera.h
#define RADIUS 1500.0f      // Initial orbit distance
#define MOVEMENTSPEED 300.0f  // Camera speed
#define SENSITIVITY 0.15f   // Mouse sensitivity
```

## Technical Highlights
```cpp
// Barnes-Hut acceleration calculation (Simulation.h)
bodies[i].acc = octree.acc(bodies[i].pos);

// Velocity-dependent coloring (main.cpp)
float speed = sqrt(sim.bodies[i].vel.x * sim.bodies[i].vel.x + 
                 sim.bodies[i].vel.y * sim.bodies[i].vel.y);
colors[i*3] = (1.0f - normalized_speed);  // Red
colors[i*3+2] = normalized_speed;         // Blue

// Galaxy initialization (utils.h)
const float outer_radius = float(std::sqrt(n)) * 5.0f;
const float vertical_scale = outer_radius * 0.1f;
```

> **Note**  
> Requires modern GPU with OpenGL 4.6 support. Start with N=5000 particles for low-end systems.
