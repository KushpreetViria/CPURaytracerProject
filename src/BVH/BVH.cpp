#include "BVH.h"
#include <algorithm>

BVH::BVH(Scene* scene)
{
	int currAxis = 0;
	std::vector<BVHBoundingBox*> objectBoundingBoxes;
	for (auto&& object : scene->objects) {
		objectBoundingBoxes.push_back(BVHBoundingBox::constructFromObject(object, this->axes[currAxis]));
	}
	sortObjectsAlongAxis(objectBoundingBoxes, axes[currAxis]);
}

BVH::~BVH()
{
}

inline void BVH::sortObjectsAlongAxis(std::vector<BVHBoundingBox*> objectBoxes, Vector axis)
{
	auto mySort = [axis](const BVHBoundingBox*& a, const BVHBoundingBox*& b)
	{
		// True if first < second
		if (axis.x == 1.0f) {
			return a->get_center().x < b->get_center().x;
		}
		else if (axis.y == 1.0f) {
			return a->get_center().y < b->get_center().y;
		}
		else if (axis.z == 1.0f) {
			return a->get_center().z < b->get_center().z;
		}
		else {
			return false;
		}
	};

	std::sort(objectBoxes.begin(), objectBoxes.end(), mySort);
}