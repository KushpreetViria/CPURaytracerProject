#include <tuple>

#include "lightingOperations.h"
#include "geometryIntersect.h"

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

float lightingOps::calcDistanceIntensity(float dist)
{
	return (1.0f / 1.0 + 0.001 * dist + 0.00001 * dist * dist);
}

float lightingOps::calcDirectionalLightShadowIntensity(const Vertex& intersection, const Vector& normal, DirectionalLight* light, const Scene& scene, int rayCount)
{
	if (rayCount <= 0) return 1.0f;
	if (rayCount == 1) {
		auto result = rayIntersectScene(intersection, -light->direction, scene, 0.001);
		return (std::get<0>(result) >= 0.001f && ((std::get<1>(result) != NULL && std::get<1>(result)->type == "plane") || std::get<1>(result) == NULL))
			? 1.0f : 0.0f;		
	}

	int uninterceptedRays = 0;
	for (int i = 0; i < rayCount; i++) {
		Vector randomLightDir = randomVectorBy(Vector(-light->direction),-0.05f,0.05f);
		auto result = rayIntersectScene(intersection, randomLightDir, scene, 0.001);
		if (std::get<0>(result) >= -0.001f && ((std::get<1>(result) != NULL && std::get<1>(result)->type == "plane")
			|| std::get<1>(result) == NULL))
			uninterceptedRays++;
	}
	return (float)uninterceptedRays / (float)rayCount;
}

float lightingOps::calcPointLightShadowIntensity(const Vertex& intersection, PointLight* light, const Scene& scene, int rayCount)
{
	if (rayCount <= 0) return 1.0f;
	if (rayCount == 1) {
		Vector d = light->position - intersection;
		auto result = rayIntersectScene(intersection, d, scene, 0.001);
		return (std::get<0>(result) > 1.0f || std::get<0>(result) < -0.001f)
			? 1.0f : 0.0f;
	}

	int uninterceptedRays = 0;
	for (int i = 0; i < rayCount; i++) {
		Vertex randomLightPoint = randomVectorBy(Vector(light->position), -0.2f, 0.2f);
		Vector d = randomLightPoint - intersection;
		auto result = rayIntersectScene(intersection, d, scene, 0.001);
		if (std::get<0>(result) > 1.0f || std::get<0>(result) < -0.001f)
			uninterceptedRays++;
	}
	return (float)uninterceptedRays / (float)rayCount;
}

float lightingOps::calcSpotLightShadowIntensity(const Vertex& intersection, SpotLight* light, const Scene& scene, int rayCount)
{
	if (rayCount <= 0) return 1.0f;
	if (rayCount == 1) {
		Vector d = light->position - intersection;
		auto result = rayIntersectScene(intersection, d, scene, 0.001);
		return (std::get<0>(result) > 1.0f || std::get<0>(result) < -0.001f)
			? 1.0f : 0.0f;
	}

	int uninterceptedRays = 0;
	for (int i = 0; i < rayCount; i++) {
		Vertex randomLightPoint = randomVectorBy(Vector(light->position), -0.1f, 0.1f);
		Vector d = randomLightPoint - intersection;
		auto result = rayIntersectScene(intersection, d, scene, 0.001);
		if (std::get<0>(result) > 1.0f || std::get<0>(result) < -0.001f)
			uninterceptedRays++;
	}

	if (uninterceptedRays < rayCount) {
		Vector lightDirection = -glm::normalize(light->direction);
		Vector directionToLight = glm::normalize(light->position - intersection);

		float angle = glm::degrees(glm::acos(glm::dot(lightDirection, directionToLight)));

		return glm::clamp(light->cutoff / angle, 0.0f, 1.0f) * (float)uninterceptedRays / (float)rayCount;
	}
	else {
		return 1.0f;
	}
}


colour3 lightingOps::calculateAmbient(AmbientLight* light, const colour3& color, Material* material, const Vertex& intersection, const Vertex& normal, const Vertex& E)
{
    return light->color * material->ambient;
}

colour3 lightingOps::calculateDirectionalPhong(DirectionalLight * light, const colour3& color, Material * material, const Vertex& intersection, const Vertex& normal, const Vertex& E)
{
	Vector directionToLight = -glm::normalize(light->direction);
	colour3 diff = material->diffuse * glm::clamp(glm::dot(normal, directionToLight), 0.0f, 1.0f);

	Vector halfway = glm::normalize(directionToLight + E);
	colour3 spec = material->specular * glm::clamp(glm::pow(glm::min(glm::dot(normal, halfway), 5.0f), material->shininess), 0.0f, 1.0f);

	return glm::clamp((diff + spec),0.0f,1.0f) * light->color;
}

colour3 lightingOps::calculatePointPhong(PointLight* light, const colour3& color, Material* material, const Vertex& intersection, const Vertex& normal, const Vertex& E)
{
	Vector directionToLight = glm::normalize(light->position - intersection);

	colour3 diff = material->diffuse * glm::clamp(glm::dot(normal, directionToLight), 0.0f, 1.0f);
	
	Vector halfway = glm::normalize(directionToLight + E);
	colour3 spec = material->specular * glm::clamp(glm::pow(glm::max(glm::dot(normal, halfway), 0.0f), material->shininess), 0.0f, 1.0f);

	return glm::clamp((diff + spec),0.0f,1.0f) * light->color;
}

colour3 lightingOps::calculateSpotPhong(SpotLight* light, const colour3& color, Material* material, const Vertex& intersection, const Vertex& normal, const Vertex& E)
{
	Vector lightDirection = -glm::normalize(light->direction);
	Vector directionToLight = glm::normalize(light->position - intersection);

	float angle = glm::degrees(glm::acos(glm::dot(lightDirection, directionToLight)));
	
	if (angle > light->cutoff) return colour3(0,0,0);

	colour3 diff = material->diffuse * glm::clamp(material->diffuse * glm::dot(normal, lightDirection), 0.0f, 1.0f);

	Vector halfway = glm::normalize(directionToLight + E);
	colour3 spec = material->specular * glm::clamp(glm::pow(glm::max(glm::dot(normal, halfway), 0.0f), material->shininess), 0.0f, 1.0f);

	return glm::clamp((diff + spec),0.0f,1.0f) * light->color;
}

colour3 lightingOps::calcRefraction(const Vector& in, const Vector& normal, float refraction_index, Vector& refracted_vec)
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

	float under_sqrt = 1 - ((glm::pow(theta_i, 2) * (1 - glm::pow(dot_product, 2))) / glm::pow(theta_r, 2));
	return  (under_sqrt >= 0)
			? (theta_i * (in - n * dot_product) / theta_r) - (n * sqrt(under_sqrt))
			: 2 * glm::dot(n, -in) * n + in;
}