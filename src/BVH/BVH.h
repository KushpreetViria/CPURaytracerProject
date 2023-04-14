#pragma once

#include "../schema.h"
#include "BVHBinaryTree.h"
#include "BVHBoundingBox.h"

class BVH {
public:
	BVH (Scene* scene);
	~BVH ();
private:
	Vertex axes[3] = { Vector(1.0f, 0, 0), Vector(0, 1.0f, 0), Vector(0, 0, 1.0f) };
	BVHBinaryTree<BVHBoundingBox> tree;

	inline void sortObjectsAlongAxis(std::vector<BVHBoundingBox*> objectBoxes, Vector axis);
};