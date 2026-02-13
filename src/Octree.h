#pragma once
#include "Body.h"
#include <glm/glm.hpp>
#include <vector>

struct Range {
    size_t start;
    size_t end;

    Range(size_t s, size_t e) : start(s), end(e) {}

    size_t size() const { return end - start; }
};

template<typename T, typename F>
size_t partition(std::vector<T>& vec, size_t start, size_t end, F predicate) {
    if (start >= end) {
        return start;
    }

    size_t l = start;
    size_t r = end - 1;

    while (true) {
        while (l <= r && predicate(vec[l])) {
            l++;
        }

        while (l < r && !predicate(vec[r])) {
            r--;
        }

        if (l >= r) {
            return l;
        }

        std::swap(vec[l], vec[r]);
        l++;
        r--;
    }
}


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
    size_t children;    // Index of first child (0 for leaves)
    size_t next;        // Index of next node in traversal order
    glm::vec3 pos;      // Center of mass
    float mass;
    Octant octant;
    Range bodies;       // Body range in main array

    Node(size_t next, Octant octant, Range bodies_range = { 0, 0 })
        : children(0), next(next), pos(0.0f), mass(0.0f),
        octant(octant), bodies(bodies_range) {
    }

    bool is_leaf() const { return children == 0; }
    bool is_branch() const { return children != 0; }
    bool is_empty() const { return mass == 0.0f; }
};

class Octree {
public:
    float t_sq;
    float e_sq;
    std::vector<Node> nodes;
    std::vector<size_t> parents;
    size_t leaf_capacity;
    float cutoffDistance = 1000.0f;

    const size_t ROOT = 0;
    int n;

    // Modified constructor to initialize with root node
    Octree(int n, float theta, float epsilon, Octant root_octant, size_t leaf=8, float cutoffDistance=1000.0f)
        : n(n),
        t_sq(theta* theta),
        e_sq(epsilon* epsilon),
        leaf_capacity(leaf),
        cutoffDistance(cutoffDistance)   
    {
        nodes.emplace_back(0, root_octant, Range{ 0, 0 });  // Initialize root node
        nodes.reserve(n);
    }
    void clear() {
        nodes.clear();
        parents.clear();
        nodes.emplace_back(0, Octant(), Range{ 0, 0 });
    }

    size_t subdivide(size_t node_idx, std::vector<Body>& bodies, Range range, size_t split[9]) {
        auto& node = nodes[node_idx];
        const glm::vec3 center = node.octant.center;

        // Partition bodies into octants
        split[0] = range.start;
        split[8] = range.end;

        // Partition order: z -> y -> x
        auto predicate_z = [&](const Body& b) { return b.pos.z <= center.z; };
        split[4] = partition(bodies, split[0], split[8], predicate_z);

        auto predicate_y_low = [&](const Body& b) { return b.pos.y <= center.y; };
        split[2] = partition(bodies, split[0], split[4], predicate_y_low);
        split[6] = partition(bodies, split[4], split[8], predicate_y_low);

        auto predicate_x_low = [&](const Body& b) { return b.pos.x <= center.x; };
        split[1] = partition(bodies, split[0], split[2], predicate_x_low);
        split[3] = partition(bodies, split[2], split[4], predicate_x_low);
        split[5] = partition(bodies, split[4], split[6], predicate_x_low);
        split[7] = partition(bodies, split[6], split[8], predicate_x_low);

        // Create child nodes
        parents.push_back(node_idx);
        const size_t children_base = nodes.size();
        node.children = children_base;

        auto octants = node.octant.subdivide();
        size_t parent_next = node.next; // Capture the parent's next value early
        for (int i = 0; i < 8; i++) {
            size_t next = (i < 7) ? children_base + i + 1 : parent_next;
            nodes.emplace_back(next, octants[i], Range{ split[i], split[i + 1] });
        }


        return children_base;
    }

    void propagate() {
        // Bottom-up accumulation
        for (auto it = parents.rbegin(); it != parents.rend(); ++it) {
            auto& parent = nodes[*it];
            parent.pos = glm::vec3(0.0f);
            parent.mass = 0.0f;

            for (size_t i = 0; i < 8; i++) {
                auto& child = nodes[parent.children + i];
                parent.pos += child.pos * child.mass;
                parent.mass += child.mass;
            }

            if (parent.mass > 0.0f) {
                parent.pos /= parent.mass;
            }
        }
    }

    void build(std::vector<Body>& bodies) {
        clear();

        nodes[ROOT].octant = Octant(bodies);
        nodes[ROOT].bodies = { 0, bodies.size() };

        std::vector<Range> node_ranges;
        node_ranges.push_back(nodes[ROOT].bodies);

        size_t current_node = 0;
        while (current_node < nodes.size()) {
            auto& node = nodes[current_node];

            if (node.bodies.size() > leaf_capacity) {
                size_t split[9];
                subdivide(current_node, bodies, node.bodies, split);

                for (int i = 0; i < 8; i++) {
                    node_ranges.push_back({ split[i], split[i + 1] });

                }
            }
            else {
                // Compute center of mass for leaf
                node.pos = glm::vec3(0.0f);
                node.mass = 0.0f;
                for (size_t i = node.bodies.start; i < node.bodies.end; i++) {
                    node.pos += bodies[i].pos * bodies[i].mass;
                    node.mass += bodies[i].mass;
                }
                if (node.mass > 0) node.pos /= node.mass;
            }
            current_node++;
        }
        propagate();
    }

    glm::vec3 acc(glm::vec3 pos, const std::vector<Body>& bodies) const {
        glm::vec3 acceleration(0.0f);
        size_t node = ROOT;

        while (true) {
            const Node& n = nodes[node];
            const glm::vec3 d = n.pos - pos;
            const float d_sq = glm::dot(d, d);

            if (d_sq > cutoffDistance * cutoffDistance) {
                if (n.next == 0) {
                    break;
                }
                node = n.next;
                continue;
            }


            if (n.is_branch() && n.octant.size * n.octant.size < d_sq * t_sq) {

                const float denom = (d_sq + e_sq) * std::sqrt(d_sq);
                acceleration += d * (n.mass / denom);

                if (n.next == 0) {
                    break;
                }
                node = n.next;
            }
            else if (n.is_leaf()) {

                for (size_t i = n.bodies.start; i < n.bodies.end; i++) {
                    const Body& body = bodies[i];
                    const glm::vec3 d_body = body.pos - pos;
                    const float d_sq_body = glm::dot(d_body, d_body);
                    const float denom = (d_sq_body + e_sq) * std::sqrt(d_sq_body);

                    // Avoid infinity by clamping to FLT_MAX
                    float force = std::min(body.mass / denom, std::numeric_limits<float>::max());
                    acceleration += d_body * force;
                }

                if (n.next == 0) {
                    break;
                }
                node = n.next;
            }
            else {
                // Recurse into children
                node = n.children;
            }
        }

        return acceleration;
    }
};