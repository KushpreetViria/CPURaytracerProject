#pragma once

#include "../schema.h"
#include "BVHBoundingBox.h"
#include "BVHBinaryTree.h"

class BVH {
public:
	BVHBinaryTree tree = BVHBinaryTree();
	BVH() {};
	BVH (Scene& scene);
	std::tuple<float, Object*, Vector, Vertex> intersectBVH(const Vertex& e, const Vector& d, float minT, bool pick = false);
	~BVH ();
private:
	Vertex axes[3] = { Vector(1.0f, 0, 0), Vector(0, 1.0f, 0), Vector(0, 0, 1.0f) };
	std::vector<Object*> planes;
	int totalBVHObjects = 0;

	void constructBVHTree(std::vector<Object*> objects, BVHBinaryTree::Node* parent, int currAxis);
	std::vector<Object*> sortObjectsAlongAxis(std::vector<Object*> objects, Vector axis);
};