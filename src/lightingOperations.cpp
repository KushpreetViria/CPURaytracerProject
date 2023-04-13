#include <tuple>

#include "lightingOperations.h"
#include "geometryIntersect.h"

float randomBetween(float small, float big)
{
	float diff = big - small;
	return (((float)rand() / (float)RAND_MAX) * diff) + small;
}

point3 randomVectorBy(point3 initial, float small, float big) {
	point3 newVec = point3(initial);
	newVec.x += randomBetween(-0.05f, 0.05f),
	newVec.y += randomBetween(-0.05f, 0.05f),
	newVec.z += randomBetween(-0.05f, 0.05f);
	return newVec;
}

float calcDistanceIntensity(float dist) {
	return (1.0f / 1.0 + 0.001 * dist + 0.00001 * dist * dist);
}

float calcDirectionalLightShadowIntensity(point3 intersection, point3 normal, DirectionalLight* light, Scene scene, int rayCount) {
	if (rayCount <= 0) return 1.0f;
	if (rayCount == 1) {
		auto result = rayIntersectScene(intersection, -light->direction, scene, 0.001);
		return (std::get<0>(result) >= 0.001f && ((std::get<1>(result) != NULL && std::get<1>(result)->type == "plane") || std::get<1>(result) == NULL))
			? 1.0f : 0.0f;		
	}

	int uninterceptedRays = 0;
	for (int i = 0; i < rayCount; i++) {
		point3 randomLightDir = randomVectorBy(point3(-light->direction),-0.05f,0.05f);
		auto result = rayIntersectScene(intersection, randomLightDir, scene, 0.001);
		if (std::get<0>(result) >= -0.001f && ((std::get<1>(result) != NULL && std::get<1>(result)->type == "plane")
			|| std::get<1>(result) == NULL))
			uninterceptedRays++;
	}
	return (float)uninterceptedRays / (float)rayCount;
}

float calcPointLightShadowIntensity(point3 intersection, PointLight* light, Scene scene, int rayCount) {
	if (rayCount <= 0) return 1.0f;
	if (rayCount == 1) {
		point3 d = light->position - intersection;
		auto result = rayIntersectScene(intersection, d, scene, 0.001);
		return (std::get<0>(result) > 1.0f || std::get<0>(result) < -0.001f)
			? 1.0f : 0.0f;
	}

	int uninterceptedRays = 0;
	for (int i = 0; i < rayCount; i++) {
		point3 randomLightPoint = randomVectorBy(point3(light->position), -0.2f, 0.2f);
		point3 d = randomLightPoint - intersection;
		auto result = rayIntersectScene(intersection, d, scene, 0.001);
		if (std::get<0>(result) > 1.0f || std::get<0>(result) < -0.001f)
			uninterceptedRays++;
	}
	return (float)uninterceptedRays / (float)rayCount;
}

float calcSpotLightShadowIntensity(point3 intersection, SpotLight* light, Scene scene, int rayCount) {
	if (rayCount <= 0) return 1.0f;
	if (rayCount == 1) {
		point3 d = light->position - intersection;
		auto result = rayIntersectScene(intersection, d, scene, 0.001);
		return (std::get<0>(result) > 1.0f || std::get<0>(result) < -0.001f)
			? 1.0f : 0.0f;
	}

	int uninterceptedRays = 0;
	for (int i = 0; i < rayCount; i++) {
		point3 randomLightPoint = randomVectorBy(point3(light->position), -0.1f, 0.1f);
		point3 d = randomLightPoint - intersection;
		auto result = rayIntersectScene(intersection, d, scene, 0.001);
		if (std::get<0>(result) > 1.0f || std::get<0>(result) < -0.001f)
			uninterceptedRays++;
	}

	if (uninterceptedRays < rayCount) {
		point3 lightDirection = -glm::normalize(light->direction);
		point3 directionToLight = glm::normalize(light->position - intersection);

		float angle = glm::degrees(glm::acos(glm::dot(lightDirection, directionToLight)));

		return glm::clamp(light->cutoff / angle, 0.0f, 1.0f) * (float)uninterceptedRays / (float)rayCount;
	}
	else {
		return 1.0f;
	}
}


point3 calculateAmbient(AmbientLight* light, point3 color, Material* material, point3 intersection, point3 normal, point3 E)
{
    return light->color * material->ambient;
}

point3 calculateDirectionalPhong(DirectionalLight* light, point3 color, Material* material, point3 intersection, point3 normal, point3 E)
{
	point3 directionToLight = -glm::normalize(light->direction);
	point3 diff = material->diffuse * glm::clamp(glm::dot(normal, directionToLight), 0.0f, 1.0f);

	point3 halfway = glm::normalize(directionToLight + E);
	point3 spec = material->specular * glm::clamp(glm::pow(glm::min(glm::dot(normal, halfway), 5.0f), material->shininess), 0.0f, 1.0f);

	return glm::clamp((diff + spec),0.0f,1.0f) * light->color;
}

point3 calculatePointPhong(PointLight* light, point3 color, Material* material, point3 intersection, point3 normal, point3 E)
{
	point3 directionToLight = glm::normalize(light->position - intersection);

	point3 diff = material->diffuse * glm::clamp(glm::dot(normal, directionToLight), 0.0f, 1.0f);
	
	point3 halfway = glm::normalize(directionToLight + E);
	point3 spec = material->specular * glm::clamp(glm::pow(glm::max(glm::dot(normal, halfway), 0.0f), material->shininess), 0.0f, 1.0f);

	return glm::clamp((diff + spec),0.0f,1.0f) * light->color;
}

point3 calculateSpotPhong(SpotLight* light, point3 color, Material* material, point3 intersection, point3 normal, point3 E)
{
	point3 lightDirection = -glm::normalize(light->direction);
	point3 directionToLight = glm::normalize(light->position - intersection);

	float angle = glm::degrees(glm::acos(glm::dot(lightDirection, directionToLight)));
	
	if (angle > light->cutoff) return point3(0,0,0);

	point3 diff = material->diffuse * glm::clamp(material->diffuse * glm::dot(normal, lightDirection), 0.0f, 1.0f);

	point3 halfway = glm::normalize(directionToLight + E);
	point3 spec = material->specular * glm::clamp(glm::pow(glm::max(glm::dot(normal, halfway), 0.0f), material->shininess), 0.0f, 1.0f);

	return glm::clamp((diff + spec),0.0f,1.0f) * light->color;
}

point3 calcRefraction(const point3 in, point3 normal, float refraction_index, point3& refracted) {
	float theta_i, theta_r;
	point3 n;

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