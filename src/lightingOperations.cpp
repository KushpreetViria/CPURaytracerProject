#include <tuple>

#include "lightingOperations.h"
#include "geometryIntersect.h"
#include "Globals.h"

float randomBetween(float small, float big)
{
	float diff = big - small;
	return (((float)rand() / (float)RAND_MAX) * diff) + small;
}

Vector randomVectorBy(const Vector& initial, float small, float big) 
{
	Vector newVec = Vector(initial);
	newVec.x += randomBetween(-0.05f, 0.05f),
	newVec.y += randomBetween(-0.05f, 0.05f),
	newVec.z += randomBetween(-0.05f, 0.05f);
	return newVec;
}

colour3 lightingOps::loopAllSceneLightsDoLighting(const colour3& inital_colour, const Scene& scene, const Material& material, const Vertex& intersection, const Vector& normal, const Vector& E, BVH* bvh) {
	colour3 colour = colour3(inital_colour);
	for (auto&& _light : scene.lights) {
		if (_light->type == "ambient") {
			AmbientLight* light = (AmbientLight*)(_light);

			colour += lightingOps::calculateAmbient(light, colour, material, intersection, normal, E);
		}
		else if (_light->type == "directional") {
			DirectionalLight* light = (DirectionalLight*)(_light);

			float shadowIntensity = Globals::SHADOWS ? lightingOps::calcDirectionalLightShadowIntensity(intersection, normal, light, scene, bvh) : 1.0f;
			colour += shadowIntensity * (lightingOps::calculateDirectionalPhong(light, colour, material, intersection, normal, E));
		}
		else if (_light->type == "point") {
			PointLight* light = (PointLight*)(_light);

			float distanceIntensity = lightingOps::calcDistanceIntensity(glm::length(light->position - intersection));
			float shadowIntensity = Globals::SHADOWS ? lightingOps::calcPointLightShadowIntensity(intersection, light, scene, bvh) : 1.0f;

			colour += shadowIntensity * distanceIntensity * (lightingOps::calculatePointPhong(light, colour, material, intersection, normal, E));
		}
		else if (_light->type == "spot") {
			SpotLight* light = (SpotLight*)(_light);

			float distanceIntensity = lightingOps::calcDistanceIntensity(glm::length(light->position - intersection));
			float shadowIntensity = Globals::SHADOWS ? lightingOps::calcSpotLightShadowIntensity(intersection, light, scene, bvh) : 1.0f;

			colour += distanceIntensity * shadowIntensity * (lightingOps::calculateSpotPhong(light, colour, material, intersection, normal, E));
		}
	}
	return colour;
}

float lightingOps::calcDistanceIntensity(float dist)
{
	return (1.0f / 1.0f + 0.001f * dist + 0.00001f * dist * dist);
}

float lightingOps::calcDirectionalLightShadowIntensity(const Vertex& intersection, const Vector& normal, DirectionalLight* light, const Scene& scene, BVH* bvh, bool pick)
{
	if (!Globals::APPROXIMATE_SHADOWS) {
		if (pick) std::cout << "Shadow directional intersection: ";
		auto& result = (!Globals::BVH) ? rayIntersectObjects(intersection, -light->direction, scene.objects, 0.001f) : bvh->intersectBVH(intersection, -light->direction, 0.001f, pick);
		return (std::get<0>(result) >= 0.001f && ((std::get<1>(result) != NULL && std::get<1>(result)->type == "plane") || std::get<1>(result) == NULL))
			? 1.0f : 0.0f;		
	}

	int uninterceptedRays = 0;
	for (int i = 0; i < Globals::APPROXIMATE_SHADOWS_RAY_COUNT; i++) {
		Vector randomLightDir = randomVectorBy(Vector(-light->direction),-0.05f,0.05f);
		if (pick) std::cout << "Shadow directional intersection: ";
		auto& result = (!Globals::BVH) ? rayIntersectObjects(intersection, -light->direction, scene.objects, 0.001f) : bvh->intersectBVH(intersection, randomLightDir, 0.001f, pick);
		if (std::get<0>(result) >= -0.001f && ((std::get<1>(result) != NULL && std::get<1>(result)->type == "plane")
			|| std::get<1>(result) == NULL))
			uninterceptedRays++;
	}
	return (float)uninterceptedRays / (float)Globals::APPROXIMATE_SHADOWS_RAY_COUNT;
}

float lightingOps::calcPointLightShadowIntensity(const Vertex& intersection, PointLight* light, const Scene& scene, BVH* bvh, bool pick)
{
	if (!Globals::APPROXIMATE_SHADOWS) {
		Vector d = light->position - intersection;
		if (pick) std::cout << "Shadow point intersection: ";
		auto& result = (!Globals::BVH) ? rayIntersectObjects(intersection, d, scene.objects, 0.001f) : bvh->intersectBVH(intersection, d, 0.001f, pick);
		return (std::get<0>(result) > 1.0f || std::get<0>(result) < -0.001f)
			? 1.0f : 0.0f;
	}

	int uninterceptedRays = 0;
	for (int i = 0; i < Globals::APPROXIMATE_SHADOWS_RAY_COUNT; i++) {
		Vertex randomLightPoint = randomVectorBy(Vector(light->position), -0.2f, 0.2f);
		Vector d = randomLightPoint - intersection;
		if (pick) std::cout << "Shadow point intersection: ";
		auto& result = (!Globals::BVH) ? rayIntersectObjects(intersection, d, scene.objects, 0.001f) : bvh->intersectBVH(intersection, d, 0.001f, pick);
		if (std::get<0>(result) > 1.0f || std::get<0>(result) < -0.001f)
			uninterceptedRays++;
	}
	return (float)uninterceptedRays / (float)Globals::APPROXIMATE_SHADOWS_RAY_COUNT;
}

float lightingOps::calcSpotLightShadowIntensity(const Vertex& intersection, SpotLight* light, const Scene& scene, BVH* bvh, bool pick)
{
	if (!Globals::APPROXIMATE_SHADOWS) {
		Vector d = light->position - intersection;
		if (pick) std::cout << "Shadow spot intersection: ";
		auto& result = (!Globals::BVH) ? rayIntersectObjects(intersection, d, scene.objects, 0.001f) : bvh->intersectBVH(intersection, d, 0.001f, pick);
		return (std::get<0>(result) > 1.0f || std::get<0>(result) < -0.001f)
			? 1.0f : 0.0f;
	}

	int uninterceptedRays = 0;
	for (int i = 0; i < Globals::APPROXIMATE_SHADOWS_RAY_COUNT; i++) {
		Vertex randomLightPoint = randomVectorBy(Vector(light->position), -0.1f, 0.1f);
		Vector d = randomLightPoint - intersection;
		if (pick) std::cout << "Shadow spot intersection: ";
		auto& result = (!Globals::BVH) ? rayIntersectObjects(intersection, d, scene.objects, 0.001f) : bvh->intersectBVH(intersection, d, 0.001f, pick);
		if (std::get<0>(result) > 1.0f || std::get<0>(result) < -0.001f)
			uninterceptedRays++;
	}

	if (uninterceptedRays < Globals::APPROXIMATE_SHADOWS_RAY_COUNT) {
		Vector lightDirection = -glm::normalize(light->direction);
		Vector directionToLight = glm::normalize(light->position - intersection);

		float angle = glm::degrees(glm::acos(glm::dot(lightDirection, directionToLight)));

		return glm::clamp(light->cutoff / angle, 0.0f, 1.0f) * (float)uninterceptedRays / (float)Globals::APPROXIMATE_SHADOWS_RAY_COUNT;
	}
	else {
		return 1.0f;
	}
}


colour3 lightingOps::calculateAmbient(AmbientLight* light, const colour3& color, const Material& material, const Vertex& intersection, const Vertex& normal, const Vertex& E)
{
    return Globals::AMBIENT ? light->color * material.ambient : light->color;
}

colour3 lightingOps::calculateDirectionalPhong(DirectionalLight * light, const colour3& color, const Material& material, const Vertex& intersection, const Vertex& normal, const Vertex& E)
{
	Vector directionToLight = -glm::normalize(light->direction);
	colour3 diff = Globals::DIFFUSE ? material.diffuse * glm::clamp(glm::dot(normal, directionToLight), 0.0f, 1.0f) : colour3(0);

	colour3 spec = Globals::SPECULAR ? material.specular * glm::clamp(glm::pow(glm::min(glm::dot(normal, glm::normalize(directionToLight + E)), 5.0f), material.shininess), 0.0f, 1.0f)
		: colour3(0);

	return glm::clamp((diff + spec),0.0f,1.0f) * light->color;
}

colour3 lightingOps::calculatePointPhong(PointLight* light, const colour3& color, const Material& material, const Vertex& intersection, const Vertex& normal, const Vertex& E)
{
	Vector directionToLight = glm::normalize(light->position - intersection);

	colour3 diff = Globals::DIFFUSE ? material.diffuse * glm::clamp(glm::dot(normal, directionToLight), 0.0f, 1.0f) : colour3(0);
	
	colour3 spec = Globals::SPECULAR ? material.specular * glm::clamp(glm::pow(glm::max(glm::dot(normal, glm::normalize(directionToLight + E)), 0.0f), material.shininess), 0.0f, 1.0f)
		: colour3(0);

	return glm::clamp((diff + spec),0.0f,1.0f) * light->color;
}

colour3 lightingOps::calculateSpotPhong(SpotLight* light, const colour3& color, const Material& material, const Vertex& intersection, const Vertex& normal, const Vertex& E)
{
	Vector lightDirection = -glm::normalize(light->direction);
	Vector directionToLight = glm::normalize(light->position - intersection);

	float angle = glm::degrees(glm::acos(glm::dot(lightDirection, directionToLight)));
	
	if (angle > light->cutoff) return colour3(0,0,0);

	colour3 diff = Globals::DIFFUSE ? material.diffuse * glm::clamp(material.diffuse * glm::dot(normal, lightDirection), 0.0f, 1.0f) : colour3(0);
	colour3 spec = Globals::SPECULAR ? material.specular * glm::clamp(glm::pow(glm::max(glm::dot(normal, glm::normalize(directionToLight + E)), 0.0f), material.shininess), 0.0f, 1.0f)
		:colour3(0);

	return glm::clamp((diff + spec),0.0f,1.0f) * light->color;
}

Vector lightingOps::calcRefraction(const Vector& in, const Vector& normal, float refraction_index)
{
	float theta_i, theta_r;
	Vector n;

	float dot_product = glm::dot(-in, normal);
	if (dot_product >= 0) {
		theta_i = 1;
		theta_r = refraction_index;
		n = normal;
		dot_product = -dot_product;
	}
	else 
	{
		theta_i = refraction_index;
		theta_r = 1;
		n = -normal;
	}

	float under_sqrt = 1.0f - ((glm::pow(theta_i, 2.0f) * (1.0f - glm::pow(dot_product, 2.0f))) / glm::pow(theta_r, 2.0f));
	return  (under_sqrt >= 0)
			? (theta_i * (in - n * dot_product) / theta_r) - (n * sqrt(under_sqrt))
			: 2 * glm::dot(n, -in) * n + in;
}

float lightingOps::calcSchlicksApprox(const Vector& in, const Vector& normal, const Material& material)
{
	float theta_i, theta_r;
	Vector n;

	float dot_product = glm::dot(-in, normal);
	if (dot_product >= 0) {
		theta_i = 1;
		theta_r = material.refraction;
		n = normal;
		dot_product = -dot_product;
	}
	else
	{
		theta_i = material.refraction;
		theta_r = 1;
		n = -normal;
	}

	float r0 = glm::pow((theta_i - theta_r) / (theta_i + theta_r), 2.0f);
	float r = r0 + (1.0f - r0) * glm::pow(1 - glm::dot(n, -in), 5.0f);
	return  r;
}

void calcTriangleBaryCoords(const Vertex& p, const Triangle& t, float& v, float& w, float& u) {
	Vector e3 = p - t.vertices[0];

	float d20 = glm::dot(e3, t.e1);
	float d21 = glm::dot(e3, t.e2);

	v = (t.d11 * d20 - t.d01 * d21) / t.denom;
	w = (t.d00 * d21 - t.d01 * d20) / t.denom;
	u = 1.0f - v - w;
}