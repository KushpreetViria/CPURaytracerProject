#include "BVH.h"
#include <iterator>
#include <algorithm>

BVH::BVH(Scene& scene)
{
	std::vector<Object*> copyWithoutPlanes;

	std::copy_if(scene.objects.begin(), scene.objects.end(), std::back_inserter(copyWithoutPlanes), [](Object* o) {return o->type != "plane"; });
	std::copy_if(scene.objects.begin(), scene.objects.end(), std::back_inserter(this->planes), [](Object* o) {return o->type == "plane"; });

	std::vector<Object*> splitMeshObjects;
	for (auto&& obj : copyWithoutPlanes) {
		if (obj->type != "mesh") splitMeshObjects.push_back(obj);
		else {
			Mesh* m = (Mesh*)(obj);
			for (auto& triangle : m->triangles) {
				splitMeshObjects.push_back(&triangle);
			}
		}
	}

	copyWithoutPlanes.clear(); //free up mem

	totalBVHObjects = splitMeshObjects.size();

	auto rootBoundingBox = BVHBoundingBox::constructFromObject(splitMeshObjects, this->axes[0]);
	auto root = tree.insertRoot(rootBoundingBox);
	constructBVHTree(splitMeshObjects, root, 0);
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
	if (overlap >= 50.0f) {
		std::cout << std::endl << "Bad BVH Node. Detected overlap of " << overlap  << "% between bounding boxes." << std::endl;
	}
	else {
		std::cout << ".";
	}

	auto leftNode = tree.insertLeft(parent, leftBoundingBox);
	auto rightNode = tree.insertRight(parent, rightBoundingBox);
	
	constructBVHTree(leftObjs, leftNode, nextAxisIdx);
	constructBVHTree(rightObjs, rightNode, nextAxisIdx);
}

std::vector<Object*> BVH::sortObjectsAlongAxis(std::vector<Object*> objects, Vector axis)
{
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

	std::sort(objects.begin(), objects.end(), mySort);
	return objects;
}

std::tuple<float, Object*, Vector, Vertex> BVH::intersectBVH(const Vertex& e, const Vector& d, float minT, bool pick) {
	std::tuple<float, Object*, Vector, Vertex> result;
	std::get<0>(result) = std::numeric_limits<float>::infinity();

	bool res = this->tree.BVHIntersect(e, d, minT, result, pick);

	// check with inifnite planes first
	if (res == false) {
		for (auto planeObj : this->planes) {
			Plane* plane = (Plane*)(planeObj);
			planeOps::PlaneIntersectResult plane_result;
			if (planeOps::rayIntersects(e, d, plane, plane_result, minT)) {
				if (plane_result.t > minT && plane_result.t < std::get<0>(result)) {
					result = { plane_result.t, planeObj, plane_result.normal, plane_result.intersection };
				}
			}
		}
	}

	return result;
}

BVH::~BVH()
{
}