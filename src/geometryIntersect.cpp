#include "geometryIntersect.h"
#include <tuple>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/string_cast.hpp>

std::tuple<float, Object*, Vector, Vertex> rayIntersectObjects(const Vertex& e, const Vector& d, std::vector<Object*> objects, float minT) {
	float best_t = std::numeric_limits<float>::infinity();
	Object* bestObj = NULL;
	Vector bestNormal;
	Vertex bestIntersectionPoint;

	for (auto&& object : objects) {
		if (object->type == "triangle") {
			Triangle* triangle = (Triangle*)(object);
			planeOps::PlaneIntersectResult result;
			if (planeOps::raytriangleIntersect(e, d, triangle, result, minT)) {
				if (result.t > minT && result.t < best_t) {
					best_t = result.t;
					bestObj = object;
					bestIntersectionPoint = result.intersection;
					bestNormal = result.normal;
				}
			}
		}
		else if (object->type == "sphere") {
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
	glm::mat4 transformInverse = glm::inverse(sphere->transformations);
	Vertex transformedRayOrigin = transformInverse * glm::vec4(e, 1.0f);
	Vector transformedRayDir = transformInverse * glm::vec4(d, 0.0f);

	Vertex c = Vertex(0,0,0);
	float r = sphere->radius;

	Vector ec = transformedRayOrigin - c;
	float dd = glm::dot(transformedRayDir, transformedRayDir);

	float discriminant = glm::pow((glm::dot(transformedRayDir, ec)), 2.0f) - dd * (glm::dot(ec, ec) - glm::pow(r, 2.0f));

	if (discriminant < 0) {
		return false;
	}
	else {
		float sqrt = glm::sqrt(discriminant);

		float t0 = (glm::dot(-transformedRayDir, ec) + sqrt) / dd;
		float t1 = (glm::dot(-transformedRayDir, ec) - sqrt) / dd;

		if (t0 <= minT && t1 <= minT) {
			return false;
		}
		else {
			if (t0 > minT && t0 <= t1) result.t_near = t0;
			else if (t1 > minT && t1 <= t0) result.t_near = t1;
			else if (t0 > minT) result.t_near = t0;
			else result.t_near = t1;

			Vertex intersectionModelSpace = transformedRayOrigin + transformedRayDir * result.t_near;
			Vector normalModelSpace = glm::normalize(sphere->transformations * glm::vec4(intersectionModelSpace, 0));
			Vertex intersectionWorldSpace = sphere->transformations * glm::vec4(intersectionModelSpace,1);

			result.intersection_near = intersectionWorldSpace;
			result.normal_near = normalModelSpace;
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
	float denom = glm::dot(plane->normal, d);
	if (denom == 0) return false;
	float t = glm::dot(plane->normal, plane->position - e) / denom;

	if (t > minT) {
		result.intersection = e + t * d;
		result.normal = plane->normal;
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

bool planeOps::raytriangleIntersect(const Vertex& e, const Vector& d, Triangle* triangle, PlaneIntersectResult& result, float minT)
{
	glm::mat4 transformInverse = glm::inverse(triangle->parent_mesh->transformations);
	Vertex transformedRayOrigin = transformInverse * glm::vec4(e, 1.0f);
	Vector transformedRayDir = transformInverse * glm::vec4(d, 0.0f);

	Vertex a = triangle->vertices[0];
	Vertex b = triangle->vertices[1];
	Vertex c = triangle->vertices[2];

	auto normal = -glm::normalize(glm::cross((c - b), (b - a)));

	planeOps::PlaneIntersectResult planeResult;
	if (!planeOps::rayIntersects(transformedRayOrigin, transformedRayDir, normal, a, planeResult, minT)) {
		return false;
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
		if (t < minT)
		{
			return false;
		}
		else {
			result.t = t;
			result.normal = triangle->parent_mesh->transformations * glm::vec4(normal,0);
			result.intersection = triangle->parent_mesh->transformations * glm::vec4(point,1);
			return true;
		}
	}

	return false;
}

// ***************************************************************************************************************** //
// Mesh operations
// ***************************************************************************************************************** //

bool meshOps::rayIntersects(const Vertex& e, const Vector& d, Mesh* mesh, meshOps::MeshRayIntersectResult& result, float minT)
{
	glm::mat4 transformInverse = glm::inverse(mesh->transformations);
	Vertex transformedRayOrigin = transformInverse * glm::vec4(e, 1.0f);
	Vector transformedRayDir = transformInverse * glm::vec4(d, 0.0f);

	bool intersects = false;
	float bestMeshTriangleT = std::numeric_limits<float>::infinity();
	Vector bestMeshTriangleN;
	Vertex bestMeshTriangleIntersection;

	for (auto&& triangle : mesh->triangles)
	{
		Vertex a = triangle.vertices[0];
		Vertex b = triangle.vertices[1];
		Vertex c = triangle.vertices[2];

		auto normal = -glm::normalize(glm::cross((c - b), (b - a)));

		planeOps::PlaneIntersectResult planeResult;
		if (!planeOps::rayIntersects(transformedRayOrigin, transformedRayDir, normal, a, planeResult, minT)) {
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

	result.normal = mesh->transformations * glm::vec4(bestMeshTriangleN,0);
	result.intersection = mesh->transformations * glm::vec4(bestMeshTriangleIntersection,1);
	result.t = bestMeshTriangleT;

	return intersects;
}

// ***************************************************************************************************************** //
// Cylinder operations
// Infinite Cylinder intersection source:
// https://www.cl.cam.ac.uk/teaching/1999/AGraphHCI/SMAG/node2.html#eqn:rectray
// ***************************************************************************************************************** //

bool cylinderOps::rayIntersects(const Vertex& e, const Vector& d, Cylinder* cylinder, CylinderIntersectResult& result, float mint) 
{
	glm::mat4 transforminverse = glm::inverse(cylinder->transformations);
	Vertex transformedrayorigin = transforminverse * glm::vec4(e, 1.0f);
	Vector transformedraydir = transforminverse  * glm::vec4(d, 0.0f);

	// calculate the quadratic coefficients for the intersection equation
	float a = glm::pow(transformedraydir.x, 2.0f) + glm::pow(transformedraydir.z, 2.0f);
	float b = 2.0f * (transformedraydir.x * transformedrayorigin.x + transformedraydir.z * transformedrayorigin.z);
	float c = glm::pow(transformedrayorigin.x, 2.0f) + glm::pow(transformedrayorigin.z, 2.0f) - glm::pow(cylinder->radius, 2.0f);

	// solve the quadratic equation to find the intersection points
	float discriminant = b * b - 4.0f * a * c;
	
	if (discriminant < 0.0f) return false; //no solution

	float sqrtdiscriminant = glm::sqrt(discriminant);
	float t1 = (-b - sqrtdiscriminant) / (2.0f * a);
	float t2 = (-b + sqrtdiscriminant) / (2.0f * a);

	if (t1 < mint && t2 < mint) {
		return false;
	}
	if (t1 < mint || t2 < mint) {
		float maxt = glm::max(t1, t2);
		t1 = maxt;
		t2 = maxt;
	}

	result.t = std::numeric_limits<float>::infinity();

	// check if the intersection points are within the height of the cylinder
	float miny = -cylinder->height / 2.0f; float maxy = cylinder->height / 2.0f;
	Vertex p1 = transformedrayorigin + t1 * transformedraydir;

	if (p1.y <= miny || p1.y >= maxy)
	{
		// the first body intersection point is outside the height of the cylinder, check the second intersection point
		Vertex p2 = transformedrayorigin + t2 * transformedraydir;
		if (p2.y <= miny || p2.y >= maxy)
		{
			// both body intersection points are outside the height of the cylinder
			return false;
		}
		else {
			result.t = t2;
			result.intersection = cylinder->transformations * glm::vec4(p2, 1);
			result.normal = glm::normalize(cylinder->transformations * glm::vec4(p2.x, 0.0f, p2.z, 0.0f));
		}
	}
	else {
		result.t = t1;
		result.intersection = cylinder->transformations * glm::vec4(p1, 1);
		result.normal = glm::normalize(cylinder->transformations * glm::vec4(p1.x, 0.0f, p1.z, 0.0f));
	}

	Vector topcapnormal = Vector(0.0f, 1.0f, 0.0f), bottomcapnormal = Vector(0.0f, -1.0f, 0.0f);
	planeOps::PlaneIntersectResult planeresulttop, planeresultbot;

	// check if the intersection points with the caps are within the radius of the cylinder
	if (planeOps::rayIntersects(transformedrayorigin, transformedraydir, topcapnormal, Vertex(0, cylinder->height / 2.0f, 0), planeresulttop, mint))
	{
		if (glm::length(glm::vec2(planeresulttop.intersection.x, planeresulttop.intersection.z)) <= cylinder->radius && planeresulttop.t < result.t)
		{
			// the intersection point with the top cap is within the radius of the cylinder
			result.t = planeresulttop.t;
			result.intersection = cylinder->transformations * glm::vec4(planeresulttop.intersection, 1);
			result.normal = cylinder->transformations * glm::vec4(topcapnormal, 0.0f);
		}
	}
	if (planeOps::rayIntersects(transformedrayorigin, transformedraydir, bottomcapnormal, Vertex(0, -cylinder->height / 2.0f, 0), planeresultbot, mint))
	{
		if (glm::length(glm::vec2(planeresultbot.intersection.x, planeresultbot.intersection.z)) <= cylinder->radius && planeresultbot.t < result.t)
		{
			// the intersection point with the bottom cap is within the radius of the cylinder
			result.t = planeresultbot.t;
			result.intersection = cylinder->transformations * glm::vec4(planeresultbot.intersection, 1);
			result.normal = cylinder->transformations * glm::vec4(bottomcapnormal, 0.0f);
		}
	}

	return true;
}


// ***************************************************************************************************************** //
// AABB operations
// AA Box intersection source:
// ***************************************************************************************************************** //
//https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-box-intersection.html
bool AABBOps::rayIntersects(const Vertex& e, const Vector& d, BVHBoundingBox* bbox, AABBIntersectResult& result, float minT)
{
	Vector invRayDir = 1.0f / d;

	// Compute t-values for intersection with each face of AABB
	Vertex aabbMin = Vertex(bbox->get_x_min(), bbox->get_y_min(), bbox->get_z_min());
	Vertex aabbMax = Vertex(bbox->get_x_max(), bbox->get_y_max(), bbox->get_z_max());
	glm::vec3 tMin = (aabbMin - e) * invRayDir;
	glm::vec3 tMax = (aabbMax - e) * invRayDir;

	// Sort t-values to find smallest and largest values
	glm::vec3 t1 = glm::min(tMin, tMax);
	glm::vec3 t2 = glm::max(tMin, tMax);

	// Find the maximum of the minimum t-values
	float tNear = glm::max(glm::max(t1.x, t1.y), t1.z);

	// Find the minimum of the maximum t-values
	float tFar = glm::min(glm::min(t2.x, t2.y), t2.z);

	if (tFar < tNear || (tNear < minT && tFar < minT))
	{
		return false;
	}

	// If the near value is negative, the ray starts inside the AABB
	if (tNear < minT)
	{
		result.t = tFar;
	}
	else {
		result.t = tNear;
	}

	result.intersection = e + result.t * d;

	return true;
}