#pragma once

#include <functional>

template <typename T>
class BVHBinaryTree {
public:
	struct Node {
		T data;
		Node* left;
		Node* right;

		Node(T data) {
			this->data = T;
			this->left = nullptr;
			this->right = nullptr;
		}
	};

	BVHBinaryTree();

	Node* insertRoot(T data);
	Node* insertLeft(Node* parent, T data);
	Node* insertRight(Node* parent, T data);

	~BVHBinaryTree();
private:
	Node* root;
};