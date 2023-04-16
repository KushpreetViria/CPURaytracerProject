#include "BVH.h"
#include <iterator>
#include <algorithm>

BVH::BVH(Scene& scene)
{
	auto rootBoundingBox = BVHBoundingBox::constructFromObject(scene.objects, this->axes[0]);
	auto root = tree.insertRoot(rootBoundingBox);
	constructBVHTree(scene.objects, root, 0);
}

/// <summary>
/// Recursively constructs BVH tree
/// </summary>
/// <param name="objects">Objects in parent</param>
/// <param name="parent">Parent Node in tree of current iteration</param>
/// <param name="currAxis">Axis to sort on</param>
void BVH::constructBVHTree(std::vector<Object*> objects, BVHBinaryTree::Node* parent, int currAxis)
{
	if (objects.size() <= 1) {
		return;
	}

	int nextAxisIdx = (currAxis + 1) % 3;

	auto sortedObjs = sortObjectsAlongAxis(objects, axes[currAxis]);
	std::size_t const half_size = sortedObjs.size() / 2;
	std::vector<Object*> leftObjs(sortedObjs.begin(), sortedObjs.begin() + half_size);
	std::vector<Object*> rightObjs(sortedObjs.begin() + half_size, sortedObjs.end());
	
	auto leftBoundingBox = BVHBoundingBox::constructFromObject(leftObjs, this->axes[nextAxisIdx]);
	auto rightBoundingBox = BVHBoundingBox::constructFromObject(rightObjs, this->axes[nextAxisIdx]);

	float overlap = BVHBoundingBox::getBoundingBoxOverlapPercentage(*leftBoundingBox, *rightBoundingBox);
	if (overlap > 75.0f) {
		return; // the two children overlapp too much, dont bother spliting
	}

	auto leftNode = tree.insertLeft(parent, leftBoundingBox);
	auto rightNode = tree.insertRight(parent, rightBoundingBox);
	
	constructBVHTree(leftObjs, leftNode, nextAxisIdx);
	constructBVHTree(rightObjs, rightNode, nextAxisIdx);
}

std::vector<Object*> BVH::sortObjectsAlongAxis(std::vector<Object*> objects, Vector axis)
{
	// drop infinite planes froms the array
	std::vector<Object*> copyWithoutPlanes;

	std::copy_if(objects.begin(), objects.end(), std::back_inserter(copyWithoutPlanes), [](Object* o) {return o->type != "plane"; });
	std::copy_if(objects.begin(), objects.end(), std::back_inserter(this->planes), [](Object* o) {return o->type == "plane"; });

	auto mySort = [axis](const Object* a, const Object* b)
	{
		Vertex aCenter = a->transformPos;
		Vertex bCenter = b->transformPos;

		// True if first < second
		if (axis.x == 1.0f) {
			return aCenter.x < bCenter.x;
		}
		else if (axis.y == 1.0f) {
			return aCenter.y < bCenter.y;
		}
		else if (axis.z == 1.0f) {
			return aCenter.z < bCenter.z;
		}
		else {
			return false;
		}
	};

	std::sort(copyWithoutPlanes.begin(), copyWithoutPlanes.end(), mySort);
	return copyWithoutPlanes;
}

std::tuple<float, Object*, Vector, Vertex> BVH::intersectBVH(const Vertex& e, const Vector& d, float minT) {
	std::tuple<float, Object*, Vector, Vertex> result;
	std::get<0>(result) = std::numeric_limits<float>::infinity();
	
	// check with inifnite planes first
	for (auto planeObj : this->planes) {
		Plane* plane = (Plane*)(planeObj);
		planeOps::PlaneIntersectResult plane_result;
		if (planeOps::rayIntersects(e, d, plane, plane_result, minT)) {
			if (plane_result.t > minT && plane_result.t < std::get<0>(result)) {
				result = { plane_result.t, planeObj, plane_result.normal, plane_result.intersection };
			}
		}
	}

	bool res = this->tree.BVHIntersect(e, d, minT, result);
	return result;
}

BVH::~BVH()
{
}