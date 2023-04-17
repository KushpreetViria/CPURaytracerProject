#pragma once

#include <functional>
#include <stack>
#include <stdexcept>
#include <tuple>

#include "../geometryIntersect.h"

class BVHBinaryTree {
public:
	struct Node {
		BVHBoundingBox* data;
		Node* left;
		Node* right;

		Node(BVHBoundingBox* data) {
			this->data = data;
			this->left = nullptr;
			this->right = nullptr;
		}
	};

	BVHBinaryTree() {
		this->root = nullptr;
	}

	Node* insertRoot(BVHBoundingBox* data) {
		if (this->root != nullptr)
		{
			throw std::runtime_error("Root already added.");
		}
		this->root = new Node(data);
		nodeCount++;
		return this->root;
	}

	Node* insertLeft(Node* parent, BVHBoundingBox* data) {
		if (parent->left != nullptr)
		{
			throw std::runtime_error("Left child already added.");
		}
		Node* child = new Node(data);
		parent->left = child;
		nodeCount++;
		return child;
	}

	Node* insertRight(Node* parent, BVHBoundingBox* data) {
		if (parent->right != nullptr)
		{
			throw std::runtime_error("Right child already added.");
		}
		Node* child = new Node(data);
		parent->right = child;
		nodeCount++;
		return child;
	}

	bool BVHIntersect(const Vertex& e, const Vector& d, float minT, std::tuple<float, Object*, Vector, Vertex>& result, bool pick) {
		int hitsTests = 0;
		int totalIntersectionTests = 0;
		bool intersectResult = _BVHIntersect(this->root, e, d, minT, result, hitsTests, totalIntersectionTests ,pick);
		if (pick) std::cout << "BVH boxes: "<< nodeCount <<", boxes hit: " << hitsTests << ", Total intersections: " << totalIntersectionTests << ", result: " << (intersectResult ? std::get<1>(result)->type : "miss") << std::endl;
		return intersectResult;
	}

	int getNodeCount() {
		return nodeCount;
	};

	~BVHBinaryTree() {
		deleteTree(this->root);
	}
private:
	Node* root;
	int nodeCount = 0;

	bool _BVHIntersect(Node* node, const Vertex& e, const Vector& d, float minT, std::tuple<float, Object*, Vector, Vertex>& result, int& hitTests, int& totalIntersectionTests, bool pick) {
		AABBOps::AABBIntersectResult temp_result;
		bool hit = AABBOps::rayIntersects(e, d, node->data, temp_result, minT);
		hitTests++;
		totalIntersectionTests++;

		if (hit) {

			if (temp_result.t > std::get<0>(result)) {
				return false;
			}

			bool hitLeft = false;
			bool hitRight = false;
			if (node->left != nullptr) {
				hitLeft = _BVHIntersect(node->left, e, d, minT, result, hitTests, totalIntersectionTests, pick);
			}
			if (node->right != nullptr) {
				hitRight = _BVHIntersect(node->right, e, d, minT, result, hitTests, totalIntersectionTests, pick);
			}
			
			if (node->left == nullptr && node->right == nullptr) {
				auto hitResult = rayIntersectObjects(e, d, node->data->get_objects(), minT);
				totalIntersectionTests += node->data->get_objects().size();
				
				Object* object = std::get<1>(hitResult);
				if (object == nullptr)
				{
					return false;
				}
				else if (std::get<0>(hitResult) < std::get<0>(result)) 
				{
					result = hitResult;
					return true;
				}

			}

			return hitLeft || hitRight;
		}

		return false;
	}

	void deleteTree(Node* node)
	{
		if (node == nullptr) return;

		/* first delete both subtrees */
		deleteTree(node->left);
		deleteTree(node->right);

		delete node;
	}
};