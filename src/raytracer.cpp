// Ray Tracer Assignment Starting Code
// Winter 2021

// The JSON library allows you to reference JSON arrays like C++ vectors and JSON objects like C++ maps.

#include "raytracer.h"

#include "BVH/BVH.h"
#include "BVH/BVHBinaryTree.h"

#include "geometryIntersect.h"
#include "lightingOperations.h"

#include <iostream>
#include <fstream>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/vector_angle.hpp>

#include "schema.h"
#include "json2scene.h"
#include "Globals.h"

const char* PATH = "scenes/";

double fov = 60;
colour3 background_colour(0, 0, 0);

json jscene;
Scene scene;
BVH* bvh;

namespace Globals {
	bool AMBIENT = true;
	bool DIFFUSE = true;
	bool SPECULAR = true;
	bool SHADOWS = true;
	bool REFLECTIONS = true;
	bool REFRACTION = true;
	bool TRANSMISSIVE = true;
	bool SCHLICKS_APPROXIMATION = false;
	bool ANTI_ALIASING = false;
	bool ANTI_ALIAS_INFINITE_PLANES = true;
	bool BVH = true;
	bool BVH_INCLUDE_PLANES = true;
	bool APPROXIMATE_SHADOWS = false;
	int APPROXIMATE_SHADOWS_RAY_COUNT = 10;
	int RAYTRACER_DEPTH = 8;
}

/****************************************************************************/

// here are some potentially useful utility functions

json find(json& j, const std::string key, const std::string value) {
	json::iterator it;
	for (it = j.begin(); it != j.end(); ++it) {
		if (it->find(key) != it->end()) {
			if ((*it)[key] == value) {
				return *it;
			}
		}
	}
	return json();
}

glm::vec3 vector_to_vec3(const std::vector<float>& v) {
	return glm::vec3(v[0], v[1], v[2]);
}

/****************************************************************************/

void choose_scene(char const* fn) {
	if (fn == NULL) {
		fn = "g";
		std::cout << "Using default input file " << PATH << fn << ".json\n";
	}

	std::cout << "Loading scene " << fn << std::endl;

	std::string fname = PATH + std::string(fn) + ".json";
	std::fstream in(fname);
	if (!in.is_open()) {
		std::cout << "Unable to open scene file " << fname << std::endl;
		exit(EXIT_FAILURE);
	}

	in >> jscene;

	if (json_to_scene(jscene, scene) < 0) {
		std::cout << "Error in scene file " << fname << std::endl;
		exit(EXIT_FAILURE);
	}

	fov = scene.camera.field;
	background_colour = scene.camera.background;
	bvh = new BVH(scene);

	if (!Globals::BVH) std::cout << std::endl << "Note: BVH is disabled!";
	std::cout << std::endl << "Finished loading (BVH size = " << bvh->tree.getNodeCount() << "). Now tracing..." << std::endl;	
}

bool isReflective(Material material) {
	return (material.reflective.r > 0 || material.reflective.g > 0 || material.reflective.b > 0);
}

bool isTransmissive(Material material) {
	return (material.transmissive.r > 0 || material.transmissive.g > 0 || material.transmissive.b > 0);
}

colour3 recursiveRayTrace(const Vertex& e, const Vector& d, bool& hit, Object*& hitObject, bool pick, int depth) {
	if (depth > 4) {
		return background_colour;
	}

	float minT = depth == 0 ? 1 : 0.001f;
	hit = depth == 0 ? false : true;

	auto& result = (!Globals::BVH) ? rayIntersectObjects(e, d, scene.objects, minT) : bvh->intersectBVH(e, d, minT, pick);

	Object* object = std::get<1>(result);
	if (object == NULL) return background_colour;
	else hit = true;

	colour3 colour = colour3(0, 0, 0);
	float t = std::get<0>(result);
	Vector normal = std::get<2>(result);
	Vertex intersection = std::get<3>(result);
	Material& material = object->material;
	Vector E = glm::normalize(e - intersection);

	if (pick && depth ==0) std::cout << "HIT: " << object->type << ", t=" << t << ", n=" << glm::to_string(normal) << std::endl;

	colour += lightingOps::loopAllSceneLightsDoLighting(colour, scene, material, intersection, normal, E, bvh);

	// a refraction value is assumed to exist if SCHLICKS_APPROXIMATION is toggled
	float schlicks = (isReflective(material) || isTransmissive(material)) ? lightingOps::calcSchlicksApprox(d, normal, material) : 1.0f;

	if (Globals::REFLECTIONS && isReflective(material)) {
		Vector reflectedRay = 2 * glm::dot(normal, E) * normal - E;
		colour3 reflectColor = recursiveRayTrace(intersection, reflectedRay, hit, hitObject, pick, depth + 1);
		colour += reflectColor * (Globals::SCHLICKS_APPROXIMATION ? colour3(schlicks)* material.reflective : material.reflective);
	}

	if (Globals::TRANSMISSIVE && isTransmissive(material)) {
		Vector reflectedRay = (Globals::REFRACTION && material.refraction > 0) ? lightingOps::calcRefraction(d, normal, material.refraction) : d;
		colour = (colour * (1.0f - material.transmissive)) + recursiveRayTrace(intersection, reflectedRay, hit, hitObject, pick, depth + 1) * material.transmissive;
	}

	colour = glm::clamp(colour, 0.0f, 1.0f);

	hitObject = object;

	return colour;
}

bool trace(const point3& e, const point3& s, colour3& colour, Object*& objectHit, bool pick) {
	Vector d = s - e;
	bool hit = false;
	colour = recursiveRayTrace(e, d, hit, objectHit, pick, 0);
	return hit;
}

bool SingleTrace(const point3& e, const point3& s, colour3& colour, Object*& objectHit, bool pick)
{
	return false;
}
