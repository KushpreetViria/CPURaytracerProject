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

const char* PATH = "scenes/";

double fov = 60;
colour3 background_colour(0, 0, 0);

json jscene;
Scene scene;
BVH* bvh;

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
		fn = "d";
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
}

bool isReflective(Material material) {
	return (material.reflective.r > 0 || material.reflective.g > 0 || material.reflective.b > 0);
}

bool isTransmissive(Material material) {
	return (material.transmissive.r > 0 || material.transmissive.g > 0 || material.transmissive.b > 0);
}

colour3 recursiveRayTrace(const Vertex& e, const Vector& d, bool& hit, bool pick, int depth) {
	if (depth > 8) {
		return background_colour;
	}

	float minT = depth == 0 ? 1 : 0.001;
	hit = depth == 0 ? false : depth;

	auto result = bvh->intersectBVH(e, d, minT);//
	//auto result = rayIntersectObjects(e, d, scene.objects, minT);

	Object* object = std::get<1>(result);

	if (object == NULL) {
		if (depth == 0) hit = false;
		return background_colour;
	}
	else {
		hit = true;
	}

	colour3 colour = colour3(0, 0, 0);
	float t = std::get<0>(result);
	Vector normal = std::get<2>(result);
	Vertex intersection = std::get<3>(result);
	Material& material = object->material;
	Vector E = glm::normalize(e - intersection);

	if (pick) std::cout << "HIT: " << object->type << ", t=" << t << ", n=" << glm::to_string(normal) << std::endl;

	for (auto&& _light : scene.lights) {
		if (_light->type == "ambient") {
			AmbientLight* light = (AmbientLight*)(_light);

			colour += lightingOps::calculateAmbient(light, colour, &material, intersection, normal, E);
		}
		else if (_light->type == "directional") {
			DirectionalLight* light = (DirectionalLight*)(_light);

			float shadowIntensity = lightingOps::calcDirectionalLightShadowIntensity(intersection, normal, light, bvh);

			colour += shadowIntensity * (lightingOps::calculateDirectionalPhong(light, colour, &material, intersection, normal, E));
		}
		else if (_light->type == "point") {
			PointLight* light = (PointLight*)(_light);

			float distanceIntensity = lightingOps::calcDistanceIntensity(glm::length(light->position - intersection));
			float shadowIntensity = lightingOps::calcPointLightShadowIntensity(intersection, light, bvh);

			colour += shadowIntensity * distanceIntensity * (lightingOps::calculatePointPhong(light, colour, &material, intersection, normal, E));
		}
		else if (_light->type == "spot") {
			SpotLight* light = (SpotLight*)(_light);

			float distanceIntensity = lightingOps::calcDistanceIntensity(glm::length(light->position - intersection));
			float shadowIntensity = lightingOps::calcSpotLightShadowIntensity(intersection, light, bvh);

			colour += distanceIntensity * shadowIntensity * (lightingOps::calculateSpotPhong(light, colour, &material, intersection, normal, E));
		}
	}

	if (isReflective(material)) {
		Vector reflectedRay = 2 * glm::dot(normal, E) * normal - E;
		colour += (recursiveRayTrace(intersection, reflectedRay, hit, pick, depth + 1) * material.reflective);
	}

	if (isTransmissive(material)) {
		Vector reflectedRay = material.refraction > 0 ? lightingOps::calcRefraction(d, normal, material.refraction, reflectedRay) : d;
		colour = (colour * (1.0f - material.transmissive)) + recursiveRayTrace(intersection, reflectedRay, hit, pick, depth + 1) * material.transmissive;
	}

	colour = glm::clamp(colour, 0.0f, 1.0f);
	return colour;
}

bool trace(const Vertex& e, const Vertex& s, colour3& colour, bool pick) {
	Vector d = s - e;
	bool hit = false;

	if (pick) {
		//bvh->intersectBVH(e, d, 0.001);
	}

	colour = recursiveRayTrace(e, d, hit, pick, 0);
	return hit;
}