#include "geometryIntersect.h"
#include <tuple>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>
//#include <glm/gtx/norm.hpp>
//#include <glm/vector_relational.hpp>
//#include <glm/gtx/vector_angle.hpp>

std::tuple<float, Object*, Vector, Vertex> rayIntersectScene(const Vertex& e, const Vector& d, const Scene& scene, float minT) {
	float best_t = std::numeric_limits<float>::infinity();
	Object* bestObj = NULL;
	Vector bestNormal;
	Vertex bestIntersectionPoint;

	for (auto&& object : scene.objects) {
		if (object->type == "sphere") {
			Sphere* sphere = (Sphere*)(object);
			sphereOps::SphereIntersectResult result;
			if (sphereOps::rayIntersects(e, d, sphere, result, minT)) {
				if (result.t_near > minT && result.t_near < best_t) {
					best_t = result.t_near;
					bestObj = object;
					bestNormal = result.normal_near;
					bestIntersectionPoint = result.intersection_near;
				}
			}
		}
		else if (object->type == "cylinder") {
			Cylinder* cylinder = (Cylinder*)(object);
			cylinderOps::CylinderIntersectResult result;
			if (cylinderOps::rayIntersects(e, d, cylinder, result, minT)) {
				if (result.t > minT && result.t < best_t) {
					best_t = result.t;
					bestObj = object;
					bestNormal = result.normal;
					bestIntersectionPoint = result.intersection;
				}
			}
		}
		else if (object->type == "plane") {
			Plane* plane = (Plane*)(object);
			planeOps::PlaneIntersectResult result;
			if (planeOps::rayIntersects(e, d, plane, result, minT)) {
				if (result.t > minT && result.t < best_t) {
					best_t = result.t;
					bestObj = object;
					bestNormal = result.normal;
					bestIntersectionPoint = result.intersection;
				}
			}
		}
		else if (object->type == "mesh") {
			Mesh* mesh = (Mesh*)(object);
			meshOps::MeshRayIntersectResult result;
			if (meshOps::rayIntersects(e, d, mesh, result, minT)) {
				if (result.t > minT && result.t < best_t) {
					best_t = result.t;
					bestObj = object;
					bestIntersectionPoint = result.intersection;
					bestNormal = result.normal;
				}
			}
		}
	}

	return { best_t, bestObj, bestNormal, bestIntersectionPoint };
}

// ***************************************************************************************************************** //
// Sphere operations
// ***************************************************************************************************************** //

bool sphereOps::rayIntersects(const Vertex& e, const Vector& d, Sphere* sphere, sphereOps::SphereIntersectResult& result, float minT) {
	Vertex c = sphere->position;
	float r = sphere->radius;

	Vector ec = e - c;
	float dd = glm::dot(d, d);

	float discriminant = glm::sqrt(glm::pow((glm::dot(d, ec)), 2) - dd * (glm::dot(ec, ec) - glm::pow(r, 2)));

	if (discriminant < 0) {
		return false;
	}
	else {
		float t0 = (glm::dot(-d, ec) + discriminant) / dd;
		float t1 = (glm::dot(-d, ec) - discriminant) / dd;

		if (t0 <= minT && t1 <= minT) {
			return false;
		}
		else {
			if (t0 > minT && t0 <= t1) result.t_near = t0;
			else if (t1 > minT && t1 <= t0) result.t_near = t1;
			else if (t0 > minT) result.t_near = t0;
			else result.t_near = t1;

			result.intersection_near = e + result.t_near * d;

			result.normal_near = glm::normalize(result.intersection_near - c);

			return true;
		}
	}
	return false;
}

// ***************************************************************************************************************** //
// Plane operations
// ***************************************************************************************************************** //

bool planeOps::rayIntersects(const Vertex& e, const Vector& d, Plane* plane, planeOps::PlaneIntersectResult& result, float minT)
{
	Vector n = plane->normal;
	Vertex a = plane->position;

	float denom = glm::dot(n, d);

	if (denom == 0) return false;

	float t = glm::dot(n, a - e) / denom;

	if (t > minT) {
		result.intersection = e + t * d;
		result.normal = n;
		result.t = t;
		return true;
	}
	else {
		return false;
	}
}

bool planeOps::rayIntersects(const Vertex& e, const Vector& d, const Vector& n, const Vertex& a, planeOps::PlaneIntersectResult& result, float minT)
{
	float denom = glm::dot(n, d);

	if (denom == 0) return false;

	float t = glm::dot(n, a - e) / denom;

	if (t > minT) {
		result.intersection = e + t * d;
		result.normal = n;
		result.t = t;
		return true;
	}
	else {
		return false;
	}
}

// ***************************************************************************************************************** //
// Mesh operations
// ***************************************************************************************************************** //

bool meshOps::rayIntersects(const Vertex& e, const Vector& d, Mesh* mesh, meshOps::MeshRayIntersectResult& result, float minT)
{
	bool intersects = false;
	float bestMeshTriangleT = std::numeric_limits<float>::infinity();
	Vector bestMeshTriangleN;
	Vertex bestMeshTriangleIntersection;

	for (auto&& triangle : mesh->triangles)
	{
		Vertex& a = triangle.vertices[0];
		Vertex& b = triangle.vertices[1];
		Vertex& c = triangle.vertices[2];

		auto normal = -glm::normalize(glm::cross((c - b), (b - a)));

		planeOps::PlaneIntersectResult planeResult;
		if (!planeOps::rayIntersects(e, d, normal, a, planeResult, minT)) {
			continue;
		}
		float t = planeResult.t;
		Vertex point = planeResult.intersection;

		std::vector<double> inTriangleTest =
		{
			glm::dot(glm::cross((b - a), (point - a)), normal),
			glm::dot(glm::cross((c - b), (point - b)), normal),
			glm::dot(glm::cross((a - c), (point - c)), normal)
		};

		if (inTriangleTest[0] > 0 && inTriangleTest[1] > 0 && inTriangleTest[2] > 0) {
			if (t < minT || t > bestMeshTriangleT)
			{
				continue;
			}
			else {
				intersects = true;
				bestMeshTriangleT = t;
				bestMeshTriangleN = normal;
				bestMeshTriangleIntersection = point;
			}
		}
	}

	result.normal = bestMeshTriangleN;
	result.intersection = bestMeshTriangleIntersection;
	result.t = bestMeshTriangleT;

	return intersects;
}

// ***************************************************************************************************************** //
// Cylinder operations
// ***************************************************************************************************************** //

bool cylinderOps::rayIntersects(const Vertex& e, const Vector& d, Cylinder* cylinder, CylinderIntersectResult& result, float minT) {
	// Transform the ray origin and direction into the coordinate system of the cylinder
	glm::mat4 cylinderTransform(1);
	for (auto t = std::crbegin(cylinder->transformations); t != std::crend(cylinder->transformations); t++) {
		cylinderTransform *= (*t);
	}
	Vertex transformedRayOrigin = glm::inverse(cylinderTransform)* glm::vec4(e, 1.0f);
	Vector transformedRayDir = glm::inverse(cylinderTransform)* glm::vec4(d, 0.0f);

	// Calculate the quadratic coefficients for the intersection equation
	float a = transformedRayDir.x * transformedRayDir.x + transformedRayDir.z * transformedRayDir.z;
	float b = 2.0f * (transformedRayDir.x * transformedRayOrigin.x + transformedRayDir.z * transformedRayOrigin.z);
	float c = transformedRayOrigin.x * transformedRayOrigin.x + transformedRayOrigin.z * transformedRayOrigin.z - cylinder->radius * cylinder->radius;

	// Solve the quadratic equation to find the intersection points
	float discriminant = b * b - 4.0f * a * c;
	
	if (discriminant < 0.0f) return false; //no solution

	float sqrtDiscriminant = glm::sqrt(discriminant);
	float t1 = (-b - sqrtDiscriminant) / (2.0f * a);
	float t2 = (-b + sqrtDiscriminant) / (2.0f * a);

	if (t1 < minT && t2 < minT) {
		return false;
	}
	if (t1 < minT || t2 < minT) {
		float maxT = glm::max(t1, t2);
		t1 = maxT;
		t2 = maxT;
	}

	// Check if the intersection points are within the height of the cylinder
	float minY = -cylinder->height / 2.0f; float maxY = cylinder->height / 2.0f;
	Vertex p1 = transformedRayOrigin + t1 * transformedRayDir;

	if (p1.y < minY || p1.y > maxY)
	{
		// The first body intersection point is outside the height of the cylinder, Check the second intersection point
		Vertex p2 = transformedRayOrigin + t2 * transformedRayDir;
		if (p2.y < minY || p2.y > maxY)
		{
			// Both body intersection points are outside the height of the cylinder
			return false;
		}
		else {
			result.t = t2;
			result.intersection = p2;
			result.normal = glm::normalize(cylinderTransform * glm::vec4(p2.x, 0.0f, p2.z, 0.0f));
		}
	}
	else {
		result.t = t1;
		result.intersection = p1;
		result.normal = glm::normalize(cylinderTransform * glm::vec4(p1.x, 0.0f, p1.z, 0.0f));
	}

	// Check for intersection with the top and bottom caps of the cylinder
	bool intersectsTopCap = false;
	bool intersectsBottomCap = false;

	Vector topCapNormal = Vector(0.0f, 1.0f, 0.0f);
	Vector bottomCapNormal = Vector(0.0f, -1.0f, 0.0f);

	planeOps::PlaneIntersectResult planeResultTop;
	planeOps::PlaneIntersectResult planeResultBot;

	// Check if the intersection points with the caps are within the radius of the cylinder
	if (planeOps::rayIntersects(transformedRayOrigin, transformedRayDir, topCapNormal, Vertex(0, cylinder->height / 2.0f, 0), planeResultTop, minT))
	{
		if (glm::length(glm::vec2(planeResultTop.intersection.x, planeResultTop.intersection.z)) <= cylinder->radius)
		{
			// The intersection point with the top cap is within the radius of the cylinder
			result.t = planeResultTop.t;
			result.intersection = planeResultTop.intersection;
			result.normal = glm::normalize(cylinderTransform * glm::vec4(topCapNormal,0.0f));  //glm::normalize(cylinderTransform * glm::vec4(topCapNormal,0.0f));
			return true;
		}
	}
	else if (planeOps::rayIntersects(transformedRayOrigin, transformedRayDir, bottomCapNormal, Vertex(0, -cylinder->height / 2.0f, 0), planeResultBot, minT))
	{
		if (glm::length(glm::vec2(planeResultBot.intersection.x, planeResultBot.intersection.z)) <= cylinder->radius)
		{
			// The intersection point with the bottom cap is within the radius of the cylinder
			result.t = planeResultBot.t;
			result.intersection = planeResultBot.intersection;
			result.normal = glm::normalize(cylinderTransform * glm::vec4(bottomCapNormal, 0.0f));
			return true;
		}
	}

	return true;
}