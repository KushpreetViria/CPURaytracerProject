#include "binaryTree.h"
#include <stdexcept>

template<typename T>
BVHBinaryTree<T>::BVHBinaryTree()
{
	this->root = nullptr;
}

template<typename T>
BVHBinaryTree<T>::Node* BVHBinaryTree<T>::insertRoot(T data)
{
	if (this->root != nullptr) 
	{
		throw std::runtime_error("Root already added."); 
	}
	this->root = new Node(data);
	return this->root;
}

template<typename T>
BVHBinaryTree<T>::Node* BVHBinaryTree<T>::insertLeft(BVHBinaryTree<T>::Node* parent, T data)
{
	if (parent->left != nullptr)
	{
		throw std::runtime_error("Left child already added.");
	}
	Node* child = new Node(data);
	parent->left = child;
	return child;
}

template<typename T>
BVHBinaryTree<T>::Node* BVHBinaryTree<T>::insertRight(BVHBinaryTree<T>::Node* parent, T data)
{
	if (parent->right != nullptr)
	{
		throw std::runtime_error("Right child already added.");
	}
	Node* child = new Node(data);
	parent->right = child;
	return child;
}

template<typename T>
BVHBinaryTree<T>::~BVHBinaryTree()
{
}
