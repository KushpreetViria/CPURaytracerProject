#pragma once

#include "../schema.h"
#include "BVHBoundingBox.h"
#include "BVHBinaryTree.h"

class BVH {
public:
	BVHBinaryTree tree = BVHBinaryTree();
	BVH() {};
	BVH (Scene& scene);
	std::tuple<float, Object*, Vector, Vertex> intersectBVH(const Vertex& e, const Vector& d, float minT);
	~BVH ();
private:
	Vertex axes[3] = { Vector(1.0f, 0, 0), Vector(0, 1.0f, 0), Vector(0, 0, 1.0f) };
	std::vector<Object*> planes;

	void constructBVHTree(std::vector<Object*> objects, BVHBinaryTree::Node* parent, int currAxis);
	std::vector<Object*> BVH::sortObjectsAlongAxis(std::vector<Object*> objects, Vector axis);
};