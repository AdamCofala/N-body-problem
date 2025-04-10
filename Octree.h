#pragma once
#include "Body.h"
#include <glm/glm.hpp>
#include <vector>

struct Range {
    size_t start;
    size_t end;
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
	size_t children;
	size_t next;
	glm::vec3 pos;
	float mass;
	Octant octant;
    Range bodies;

	Node(size_t next, Octant octant, Range bodies_range = { 0, 0 })
		: children(0),
		next(next),
		pos({0.0f,0.0f,0.0f}),
		mass(0.0f),
		octant(octant),
        bodies(bodies_range){
	}

	bool is_leaf() const{
		return children == 0;
	}

	bool is_branch() const{
		return children != 0;
	}

	bool is_empty() const{
		return mass == 0.0f;
	}
};

class Octree {
public:
    float t_sq;
    float e_sq;
    std::vector<Node> nodes;
    std::vector<size_t> parents;
    size_t leaf_capacity;

    const size_t ROOT = 0;
    int n = 20000;

    // Modified constructor to initialize with root node
    Octree(float theta, float epsilon, Octant root_octant, size_t leaf)
        : t_sq(theta* theta),
        e_sq(epsilon* epsilon),
        leaf_capacity(leaf)
    {
        nodes.emplace_back(0, root_octant);  // Initialize root node
        nodes.reserve(n*0.5);
    }
    void clear(Octant octant) {
        nodes.clear();
        parents.clear();
        nodes.emplace_back(0, octant);  // Reset to root node
    }

    size_t subdivide(size_t node, std::vector<Body>& bodies, Range range, size_t split[9]) {
        const glm::vec3 center = nodes[node].octant.center;

        split[0] = range.start;
        split[8] = range.end;

        // Predicates must match findOctant() logic (using >)
        auto predicate_z = [&center](const Body& body) { return body.pos.z <= center.z; };
        auto predicate_y = [&center](const Body& body) { return body.pos.y <= center.y; };
        auto predicate_x = [&center](const Body& body) { return body.pos.x <= center.x; };

        // Partition by z-coordinate first
        split[4] = partition(bodies, split[0], split[8], predicate_z);

        // Partition by y-coordinate
        split[2] = partition(bodies, split[0], split[4], predicate_y);
        split[6] = partition(bodies, split[4], split[8], predicate_y);

        // Partition by x-coordinate
        split[1] = partition(bodies, split[0], split[2], predicate_x);
        split[3] = partition(bodies, split[2], split[4], predicate_x);
        split[5] = partition(bodies, split[4], split[6], predicate_x);
        split[7] = partition(bodies, split[6], split[8], predicate_x);

        // Only subdivide if there are enough bodies
        if (range.size() <= leaf_capacity) {
            return 0; // No subdivision needed
        }

        parents.push_back(node);
        const size_t children = nodes.size();
        nodes[node].children = children;

        std::vector<Octant> octants = nodes[node].octant.subdivide();
        for (size_t i = 0; i < 8; i++) {
            const size_t next = (i < 7) ? children + i + 1 : nodes[node].next;
            nodes.emplace_back(next, octants[i], Range{ split[i], split[i + 1] });
        }

        return children;
    }

    void propagate() {
        // First pass: Accumulate masses and weighted positions
        for (auto it = parents.rbegin(); it != parents.rend(); ++it) {
            size_t node = *it;
            size_t child = nodes[node].children;

            // Reset mass and position for accumulation
            nodes[node].pos = glm::vec3(0.0f);
            nodes[node].mass = 0.0f;

            // For octrees we need to sum all 8 children
            for (size_t i = 0; i < 8; ++i) {
                nodes[node].pos += nodes[child + i].pos * nodes[child + i].mass;
                nodes[node].mass += nodes[child + i].mass;
            }
        }

        // Second pass: Normalize positions
        for (auto& node : nodes) {
            // Avoid division by zero by using a small positive value
            float divisor = std::max(node.mass, std::numeric_limits<float>::min());
            node.pos /= divisor;
        }
    }

    void build(std::vector<Body>& bodies, size_t leaf_capacity = 8) {
        // First, clear the tree and initialize with root node
        Octant root_octant(bodies);
        nodes.clear();
        parents.clear();
        nodes.emplace_back(0, root_octant, Range{ 0, bodies.size() });

        std::vector<Range> node_bodies;
        node_bodies.push_back({ 0, bodies.size() });

        // Process each node
        size_t node = 0;
        while (node < nodes.size()) {
            Range range = node_bodies[node];

            // Subdivide if needed
            if (range.size() > leaf_capacity) {
                size_t split[9];
                size_t children = subdivide(node, bodies, range, split);

                // Add ranges for each child using the split from subdivide
                for (size_t i = 0; i < 8; i++) {
                    node_bodies.push_back({ split[i], split[i + 1] });
                }
            }
            else {
                // Calculate center of mass for leaf nodes
                nodes[node].pos = glm::vec3(0.0f);
                nodes[node].mass = 0.0f;

                for (size_t i = range.start; i < range.end; i++) {
                    nodes[node].pos += bodies[i].pos * bodies[i].mass;
                    nodes[node].mass += bodies[i].mass;
                }
            }

            node++;
        }

        // Finalize by propagating values up the tree
        propagate();
    }

    glm::vec3 acc(glm::vec3 pos, const std::vector<Body>& bodies) const {
        glm::vec3 acceleration(0.0f);
        size_t node = ROOT;

        while (true) {
            const Node& n = nodes[node];
            const glm::vec3 d = n.pos - pos;
            const float d_sq = glm::dot(d, d);

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