#include "BVHBoundingBox.h"

// ifinite objects, like planes are not bounded!!!
BVHBoundingBox* BVHBoundingBox::constructFromObject(std::vector<Object*> objects, Vector BVHSortingAxis)
{
	constexpr float infinity = std::numeric_limits<float>::infinity();
	float minX = infinity, minY = infinity, minZ = infinity;
	float maxX = -infinity, maxY = -infinity, maxZ = -infinity;
	bool exists = false;

	for (auto&& obj : objects) {
		glm::mat4 transformations = getTransformations(obj);

		if (obj->type == "sphere") {
			exists = true;
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
				minValues.x = glm::min(minValues.x, vertex.x);
				minValues.y = glm::min(minValues.y, vertex.y);
				minValues.z = glm::min(minValues.z, vertex.z);

				maxValues.x = glm::max(maxValues.x, vertex.x);
				maxValues.y = glm::max(maxValues.y, vertex.y);
				maxValues.z = glm::max(maxValues.z, vertex.z);
			}

			minX = glm::min(minX, minValues.x); minY = glm::min(minY, minValues.y); minZ = glm::min(minZ, minValues.z);
			maxX = glm::max(maxX, maxValues.x); maxY = glm::max(maxY, maxValues.y); maxZ = glm::max(maxZ, maxValues.z);

		}
		else if (obj->type == "cylinder") {
			exists = true;
			Cylinder* cylinder = (Cylinder*)(obj);
			std::vector<Vertex> vertices;
			float halfHeight = cylinder->height / 2.0f;

			// source:
			// https://iquilezles.org/articles/diskbbox/

			Vertex pa = transformations * glm::vec4(0, halfHeight, 0,1);
			Vertex pb = transformations * glm::vec4(0, -halfHeight, 0,1);

			Vector a = pb - pa;
			Vector e = cylinder->radius * sqrt(1.0f - a * a / glm::dot(a, a));
			vertices.push_back(glm::vec4(pa - e,1));
			vertices.push_back(glm::vec4(pb - e,1));
			vertices.push_back(glm::vec4(pa + e,1));
			vertices.push_back(glm::vec4(pb + e,1));
			

			Vertex minValues = vertices[0];
			Vertex maxValues = vertices[0];
			for (auto& vertex : vertices) {
				minValues.x = glm::min(minValues.x, vertex.x);
				minValues.y = glm::min(minValues.y, vertex.y);
				minValues.z = glm::min(minValues.z, vertex.z);

				maxValues.x = glm::max(maxValues.x, vertex.x);
				maxValues.y = glm::max(maxValues.y, vertex.y);
				maxValues.z = glm::max(maxValues.z, vertex.z);
			}

			minX = glm::min(minX, minValues.x); minY = glm::min(minY, minValues.y); minZ = glm::min(minZ, minValues.z);
			maxX = glm::max(maxX, maxValues.x); maxY = glm::max(maxY, maxValues.y); maxZ = glm::max(maxZ, maxValues.z);
		}
		else if (obj->type == "mesh") {
			exists = true;
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
				minValues.x = glm::min(minValues.x, vertex.x);
				minValues.y = glm::min(minValues.y, vertex.y);
				minValues.z = glm::min(minValues.z, vertex.z);

				maxValues.x = glm::max(maxValues.x, vertex.x);
				maxValues.y = glm::max(maxValues.y, vertex.y);
				maxValues.z = glm::max(maxValues.z, vertex.z);
			}

			minX = glm::min(minX, minValues.x); minY = glm::min(minY, minValues.y); minZ = glm::min(minZ, minValues.z);
			maxX = glm::max(maxX, maxValues.x); maxY = glm::max(maxY, maxValues.y); maxZ = glm::max(maxZ, maxValues.z);
		}
	}

	if (exists) {
		return new BVHBoundingBox(BVHSortingAxis, objects, minX, minY, minZ, maxX, maxY, maxZ);
	}
	else {
		return nullptr;
	}
}

float BVHBoundingBox::getBoundingBoxOverlapPercentage(const BVHBoundingBox& a, const BVHBoundingBox& b) {
	float overlapX = std::max(0.0f, std::min(a.get_x_max(), b.get_x_max()) - std::max(a.get_x_min(), b.get_x_min()));
	float overlapY = std::max(0.0f, std::min(a.get_y_max(), b.get_y_max()) - std::max(a.get_y_min(), b.get_y_min()));
	float overlapZ = std::max(0.0f, std::min(a.get_z_max(), b.get_z_max()) - std::max(a.get_z_min(), b.get_z_min()));
	float overlapVolume = overlapX * overlapY * overlapZ;
	float totalVolume = a.get_width() * a.get_height() * a.get_depth() + b.get_width() * b.get_height() * b.get_depth() - overlapVolume;
	float overlapPercentage = overlapVolume / totalVolume * 100.0f;
	return overlapPercentage;
}


BVHBoundingBox::~BVHBoundingBox()
{}

glm::mat4 BVHBoundingBox::getTransformations(Object*& obj)
{
	glm::mat4 transformations(1);
	std::vector<glm::mat4> transformationMatrices;
	if (obj->type == "cylinder") {
		Cylinder* cylinder = (Cylinder*)(obj);
		transformationMatrices = cylinder->transformations;
	}
	else if (obj->type == "sphere") {
		Sphere* sphere = (Sphere*)(obj);
		transformationMatrices = sphere->transformations;
	}
	else if (obj->type == "mesh") {
		Mesh* mesh = (Mesh*)(obj);
		transformationMatrices = mesh->transformations;
	}

	for (auto t = std::crbegin(transformationMatrices); t != std::crend(transformationMatrices); t++) {
		transformations *= (*t);
	}

	return transformations;
}
