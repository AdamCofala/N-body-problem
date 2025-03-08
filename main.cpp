#include <iostream>
#include <iomanip>
#include "Body.h"  // Assuming your Body class is in Body.h
#include <string>

// Helper function to print vector components
void printVec3(const glm::vec3& v, const std::string& name) {
    std::cout << name << ": ("
        << std::fixed << std::setprecision(2) << v.x << ", "
        << std::fixed << std::setprecision(2) << v.y << ", "
        << std::fixed << std::setprecision(2) << v.z << ")" << std::endl;
}

// Helper function to print body state
void printBodyState(const Body& body, const std::string& label) {
    std::cout << "--- " << label << " ---" << std::endl;
    printVec3(body.pos, "Position");
    printVec3(body.vel, "Velocity");
    printVec3(body.acc, "Acceleration");
    std::cout << "Mass: " << body.mass << std::endl;
    std::cout << "Radius: " << body.radius << std::endl;
    std::cout << std::endl;
}

int main() {
    // Test 1: Default constructor
    std::cout << "=== Test 1: Default Constructor ===" << std::endl;
    Body body1;
    printBodyState(body1, "Default Body");

    // Test 2: Parameterized constructor
    std::cout << "=== Test 2: Parameterized Constructor ===" << std::endl;
    Body body2(glm::vec3(1.0f, 2.0f, 3.0f), glm::vec3(0.5f, 0.5f, 0.5f), 10.0f, 2.0f);
    printBodyState(body2, "Custom Body");

    // Test 3: Basic update with zero acceleration
    std::cout << "=== Test 3: Basic Update with Zero Acceleration ===" << std::endl;
    Body body3(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 2.0f, 3.0f), 5.0f, 1.5f);
    printBodyState(body3, "Before Update");

    // Update with dt = 1.0
    body3.update(1.0f);
    printBodyState(body3, "After Update (dt=1.0)");

    // Test 4: Update with acceleration
    std::cout << "=== Test 4: Update with Acceleration ===" << std::endl;
    Body body4(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), 5.0f, 1.5f);
    // Add some acceleration
    body4.acc = glm::vec3(0.0f, -9.8f, 0.0f);  // Like gravity
    printBodyState(body4, "Before Update");

    // Update with dt = 0.5
    body4.update(0.5f);
    printBodyState(body4, "After First Update (dt=0.5)");

    // Add some more acceleration and update again
    body4.acc = glm::vec3(1.0f, -9.8f, 0.0f);
    body4.update(0.5f);
    printBodyState(body4, "After Second Update (dt=0.5)");

    // Test 5: Multiple updates over time
    std::cout << "=== Test 5: Multiple Updates Over Time ===" << std::endl;
    Body body5(glm::vec3(0.0f, 10.0f, 0.0f), glm::vec3(2.0f, 0.0f, 0.0f), 5.0f, 1.0f);
    printBodyState(body5, "Initial State");

    // Simulate 5 time steps with constant gravity
    float dt = 0.1f;
    for (int i = 1; i <= 5; i++) {
        body5.acc = glm::vec3(0.0f, -9.8f, 0.0f);
        body5.update(dt);
        printBodyState(body5, "Time step " + std::to_string(i));
    }

    return 0;
}