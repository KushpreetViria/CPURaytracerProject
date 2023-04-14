#include "BVHBoundingBox.h"

// ifinite objects, like planes are not bounded!!!
BVHBoundingBox* BVHBoundingBox::constructFromObject(Object* obj, Vector BVHSortingAxis)
{
	glm::mat4 transformations = getTransformations(obj);

	if (obj->type == "sphere") {
		Sphere* sphere = (Sphere*)(obj);
		std::vector<Vertex> vertices;
		vertices.push_back(transformations * glm::vec4((Vertex(sphere->radius, 0, 0)), 1.0f));
		vertices.push_back(transformations * glm::vec4((Vertex(-sphere->radius, 0, 0)), 1.0f));
		vertices.push_back(transformations * glm::vec4((Vertex(0, sphere->radius, 0)), 1.0f));
		vertices.push_back(transformations * glm::vec4((Vertex(0, -sphere->radius, 0)), 1.0f));
		vertices.push_back(transformations * glm::vec4((Vertex(0, 0, sphere->radius)), 1.0f));
		vertices.push_back(transformations * glm::vec4((Vertex(0, 0, -sphere->radius)), 1.0f));

		Vertex minValues = vertices[0];
		Vertex maxValues = vertices[0];
		for (auto& vertex : vertices) {
			minValues = glm::min(minValues, vertex);
			maxValues = glm::max(maxValues, vertex);
		}

		return new BVHBoundingBox(BVHSortingAxis, minValues.x, minValues.y, minValues.z, maxValues.x, maxValues.y, maxValues.z);
	}
	else if (obj->type == "cylinder") {
		Cylinder* cylinder = (Cylinder*)(obj);
		std::vector<Vertex> vertices;
		vertices.push_back(transformations * glm::vec4((Vertex(0, cylinder->height, 0)), 1.0f));
		vertices.push_back(transformations * glm::vec4((Vertex(0, -cylinder->height, 0)), 1.0f));
		vertices.push_back(transformations * glm::vec4((Vertex(cylinder->radius, 0, 0)), 1.0f));
		vertices.push_back(transformations * glm::vec4((Vertex(-cylinder->radius, 0, 0)), 1.0f));
		vertices.push_back(transformations * glm::vec4((Vertex(0, 0, cylinder->radius)), 1.0f));
		vertices.push_back(transformations * glm::vec4((Vertex(0, 0, -cylinder->radius)), 1.0f));

		Vertex minValues = vertices[0];
		Vertex maxValues = vertices[0];
		for (auto& vertex : vertices) {
			minValues = glm::min(minValues, vertex);
			maxValues = glm::max(maxValues, vertex);
		}

		return new BVHBoundingBox(BVHSortingAxis, minValues.x, minValues.y, minValues.z, maxValues.x, maxValues.y, maxValues.z);
	}
	else if (obj->type == "mesh") {
		Mesh* mesh = (Mesh*)(obj);
		std::vector<Vertex> vertices;

		for (auto&& triangle : mesh->triangles) {
			vertices.push_back(transformations * glm::vec4(triangle.vertices[0], 1));
			vertices.push_back(transformations * glm::vec4(triangle.vertices[1], 1));
			vertices.push_back(transformations * glm::vec4(triangle.vertices[2], 1));
		}

		Vertex minValues = vertices[0];
		Vertex maxValues = vertices[0];
		for (auto& vertex : vertices) {
			minValues = glm::min(minValues, vertex);
			maxValues = glm::max(maxValues, vertex);
		}

		return new BVHBoundingBox(BVHSortingAxis, minValues.x, minValues.y, minValues.z, maxValues.x, maxValues.y, maxValues.z);
	}

	return nullptr;
}

BVHBoundingBox::~BVHBoundingBox()
{}

glm::mat4 BVHBoundingBox::getTransformations(Object*& obj)
{
	glm::mat4 transformations(1);
	if (obj->type == "cylinder") {
		Cylinder* cylinder = (Cylinder*)(obj);
		for (auto t = std::crbegin(cylinder->transformations); t != std::crend(cylinder->transformations); t++) {
			transformations *= (*t);
		}
	}

	return transformations;
}
