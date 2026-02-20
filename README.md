# N-Body Simulation

## 1. Project Overview

| <video src="https://github.com/user-attachments/assets/fc3275ba-f3b1-438c-9383-7e62a2d7a295" width="300"></video> | Real-time 3D N-body gravity simulation with Barnes-Hut octree optimization and OpenGL rendering. Built with C++17, OpenGL 4.6, GLFW, GLM, and GLAD.|
| --- | --- |


**Key Features:**
- Barnes-Hut octree for O(n log n) force calculations
- OpenGL 4.6 rendering with point sprites and alpha blending
- Interactive 3D camera (orbit, zoom, height, FOV)
- Velocity-based and mass-based coloring modes
- Dynamic particle sizing based on mass
- Single and double galaxy initialization
- Multi-threaded with OpenMP (optional)
- Pause/resume simulation
- FPS display in title bar

| ![image](https://github.com/user-attachments/assets/ee677e8a-0486-4b43-9d15-909dde6807a8) | ![image](https://github.com/user-attachments/assets/1c9ad6cf-79cf-45d9-bf6f-4a2e37430422) | ![image](https://github.com/user-attachments/assets/e5a6a5b3-f657-459e-be73-611edd1b6e70) |
|---|---|---|
| Full galaxy overview | Double galaxy option | Velocity and mass based color/size mapping |

---

## 2. Architecture

### 2.1. `Body` Class (`Body.h`)
Represents a single body with position, velocity, acceleration, mass, and radius. Provides velocity-based and mass-based coloring via `getColor()`.

### 2.2. `Octree` Class (`Octree.h`)
Implements the Barnes-Hut octree for spatial partitioning. Recursively subdivides 3D space into octants, approximating distant groups as single center-of-mass nodes. Computes gravitational acceleration for each body in O(n log n).

### 2.3. `Simulation` Class (`Simulation.h`)
Core simulation manager. Initializes bodies (single or double galaxy), builds the octree each step, computes gravitational forces, and integrates positions/velocities with configurable timestep.

### 2.4. `Camera` Class (`Camera.h`)
Orbital camera with mouse-driven rotation, scroll zoom, keyboard height/position control, and dynamic FOV adjustment. Outputs view and projection matrices for the renderer.

### 2.5. `utils.h`
Galaxy initialization functions. `uniform_disc()` generates a single disc-shaped galaxy with a central massive body. `uniform_disc_bin()` generates a double galaxy system. Bodies are placed with radial distribution and Keplerian orbital velocities.

### 2.6. `main.cpp`
Application entry point. Initializes GLFW window with OpenGL 4.6 Core Profile, compiles vertex/fragment shaders, manages GPU buffers (VAO, VBO for positions, colors, sizes), runs the main loop (poll events → simulate → render), handles input callbacks, and cleans up resources.

**Shaders:**
- Vertex: Transforms positions via view/projection matrices, passes per-particle color and size
- Fragment: Draws smooth circles with `smoothstep` alpha and additive blending

---

## 3. User Guide

### 3.1. Requirements
- CMake 3.16+
- C++17 compiler (MSVC recommended on Windows)
- OpenGL 4.6 capable GPU/driver
- GLFW 3.3 (via vcpkg or system install)

### 3.2. Building with CMake

#### Visual Studio (recommended)

1. Install GLFW via vcpkg:
   ```bash
   vcpkg install glfw3:x64-windows
   ```
2. Open the project folder in Visual Studio (File → Open → Folder).
3. Visual Studio detects `CMakeLists.txt` and configures the project automatically.
   - If using vcpkg, ensure the toolchain is set: in `CMakeSettings.json` or via the `VCPKG_ROOT` environment variable.
4. Select the `Nbody` target, build and run.

#### Command line

```bash
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake
cmake --build build --config Release
./build/Release/Nbody.exe
```

### 3.3. Controls

| Key          | Action                     |
|--------------|----------------------------|
| `W/A/S/D`   | Move camera                |
| `Q/E`       | Move camera target down/up |
| `R`         | Toggle mouse cursor        |
| `P`         | Pause/resume simulation    |
| `ESC`       | Exit                       |
| Mouse drag  | Rotate view                |
| Scroll      | Zoom in/out                |

### 3.4. Configuration

**Simulation settings** (`main.cpp`):
```cpp
const int N         = 30000;          // Number of bodies
const int type      = 1;              // 0 = single galaxy, 1 = double galaxy
const int typeColor = 0;              // 0 = velocity-based, 1 = mass-based
```

**Camera defaults** (`Camera.h`):
```cpp
#define SENSITIVITY 0.08f
#define ZOOM        70.0f
```

**Simulation parameters** (`Simulation.h`):
```cpp
float theta          = 1.0f;          // Barnes-Hut opening angle
float epsilon        = 0.1f;          // Softening factor
unsigned int leaf_capacity = 4;       // Max bodies per octree leaf
float cutoffDistance = 1500.0f;       // Force cutoff distance
```

---

## 4. Reflection

### 4.1. Completed Work
- Barnes-Hut octree with configurable theta and cutoff distance
- Two galaxy initialization modes (single and binary)
- Velocity-based and mass-based coloring with smooth gradients
- Mass-proportional particle sizing
- Interactive 3D orbital camera with full keyboard/mouse controls
- OpenMP parallelization for buffer updates
- Modern OpenGL 4.6 rendering with custom shaders
- Pause/resume functionality
- Dynamic window resizing with aspect ratio adaptation

### 4.2. Future Enhancements
- GUI panel (ImGui) for runtime parameter tuning
- GPU compute shaders for force calculation
- Collision detection and merging
- Save/load simulation state
- Trail/trajectory visualization
- Additional galaxy shapes (spiral arms, elliptical)

---

## 5. References

- [The Barnes-Hut Algorithm - Tom Ventimiglia & Kevin Wayne](https://arborjs.org/docs/barnes-hut)
- [Barnes-Hut Galaxy Simulator - cs.princeton.edu](https://www.cs.princeton.edu/courses/archive/fall03/cs126/assignments/barnes-hut.html)
- [A Parallel Tree Code - John Dubinski](https://arxiv.org/pdf/astro-ph/9603097)
- [Learn OpenGL](https://learnopengl.com/)
- [OpenGL Reference Pages](https://registry.khronos.org/OpenGL-Refpages/)

