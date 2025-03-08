#pragma once
#include "Body.h"
#include <glm/glm.hpp>
#include <vector>


class Octant {
public:
	glm::vec3 center;
	float size;

	Octant() : center(0.0f, 0.0f, 0.0f), size(0.0f) {}
	Octant(std::vector<Body>& Bodies) {

		float min_x = FLT_MAX;
		float max_x = FLT_MIN;
		float min_y = FLT_MAX;
		float max_y = FLT_MIN;
		float min_z = FLT_MAX;
		float max_z = FLT_MIN;

		for (const Body &body : Bodies) {

			min_x = std::min(min_x, body.pos.x);
			max_x = std::max(max_x, body.pos.x);
			min_y = std::min(min_y, body.pos.y);
			max_y = std::max(max_y, body.pos.y);
			min_z = std::min(min_z, body.pos.z);
			max_z = std::max(max_z, body.pos.z);
		}

		center = glm::vec3(min_x + max_x, min_y + max_y, min_z + max_z) * 0.5f;

		float width = max_x - min_x;
		float height = max_y - min_y;
		float depth = max_z - min_z;
		size = std::max(std::max(width, height), depth);

	}

	unsigned int findOctant(const glm::vec3& pos) const {
		return ((pos.z > center.z) << 2) |
			((pos.y > center.y) << 1) |
			(pos.x > center.x);
	}

	Octant intoOctant(unsigned int octant) const {
		Octant result = *this;
		result.size *= 0.5f;

		result.center.x += ((octant & 1) - 0.5f) * result.size;
		result.center.y += (((octant >> 1) & 1) - 0.5f) * result.size;
		result.center.z += ((octant >> 2) - 0.5f) * result.size;

		return result;
	}

	std::vector<Octant> subdivide() const {
		std::vector<Octant> children(8);
		for (unsigned int i = 0; i < 8; i++) {
			children[i] = intoOctant(i);
		}
		return children;
	}
};

class Node {
public: 
	size_t children;
	size_t next;
	glm::vec3 pos;
	float mass;
	Octant octant;

	Node(size_t next, Octant octant)
		: children(0),
		next(next),
		pos({0.0f,0.0f,0.0f}),
		mass(0.0f),
		octant(octant) {
	}

	bool is_leaf() {
		return children == 0;
	}

	bool is_branch() {
		return children != 0;
	}

	bool is_empty() {
		return mass == 0.0f;
	}
};

class Octree {
public:
    float t_sq;
    float e_sq;
    std::vector<Node> nodes;
    std::vector<size_t> parents;
    const size_t ROOT = 0;

    // Modified constructor to initialize with root node
    Octree(float theta, float epsilon, Octant root_octant)
        : t_sq(theta* theta),
        e_sq(epsilon* epsilon)
    {
        nodes.emplace_back(0, root_octant);  // Initialize root node
    }
    void clear(Octant octant) {
        nodes.clear();
        parents.clear();
        nodes.emplace_back(0, octant);  // Reset to root node
    }

    size_t subdivide(size_t node) {
        parents.push_back(node);
        const size_t children = nodes.size();
        nodes[node].children = children;

        std::vector<Octant> octants = nodes[node].octant.subdivide();

        // Create 8 children with proper next pointers
        for (size_t i = 0; i < 8; i++) {
            const size_t next = (i < 7) ? children + i + 1 : nodes[node].next;
            nodes.emplace_back(next, octants[i]);
        }

        return children;
    }

    void propagate() {
        for (auto it = parents.rbegin(); it != parents.rend(); ++it) {
            size_t node = *it;
            size_t child = nodes[node].children;

            glm::vec3 pos_sum(0.0f);
            float mass_sum = 0.0f;

            for (size_t i = 0; i < 8; ++i) {
                pos_sum += nodes[child + i].pos * nodes[child + i].mass;
                mass_sum += nodes[child + i].mass;
            }

            nodes[node].pos = pos_sum / mass_sum;
            nodes[node].mass = mass_sum;
        }
    }

    void insert(glm::vec3 pos, float mass) {
        size_t node = ROOT;

        while (true) {
            Node& current = nodes[node];

            if (current.is_branch()) {
                const unsigned octant = current.octant.findOctant(pos);
                node = current.children + octant;

                // Bounds check
                if (node >= nodes.size()) {
                    throw std::out_of_range("Invalid node access");
                }
            }
            else {
                if (current.is_empty()) {
                    current.pos = pos;
                    current.mass = mass;
                    return;
                }

                // Handle collision/duplicate
                if (pos == current.pos) {
                    current.mass += mass;
                    return;
                }

                // Store existing data
                const glm::vec3 old_pos = current.pos;
                const float old_mass = current.mass;

                while (true) {
                    const size_t children = subdivide(node);
                    const unsigned q1 = nodes[node].octant.findOctant(old_pos);
                    const unsigned q2 = nodes[node].octant.findOctant(pos);

                    if (q1 != q2) {
                        nodes[children + q1].pos = old_pos;
                        nodes[children + q1].mass = old_mass;
                        nodes[children + q2].pos = pos;
                        nodes[children + q2].mass = mass;
                        return;
                    }
                    node = children + q1;
                }
            }
        }
    }

    glm::vec3 acc(glm::vec3 pos) const {
        glm::vec3 acceleration(0.0f);
        size_t node = ROOT;

        while (node < nodes.size()) {
            Node n = nodes[node];
            const glm::vec3 d = n.pos - pos;
            const float d_sq = glm::dot(d, d);
            const float size_sq = n.octant.size * n.octant.size;

            if (n.is_leaf() || (size_sq / d_sq < t_sq)) {
                const float denom = d_sq + e_sq;
                acceleration += n.mass * d / (denom * std::sqrt(denom));

                if (n.next == 0) break;
                node = n.next;
            }
            else {
                node = n.children;
            }
        }
        return acceleration;
    }
};