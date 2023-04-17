#pragma once

#include <glm/glm.hpp>
#include "schema.h"
#include "BVH/BVH.h"

typedef glm::vec3 point3;
typedef glm::vec3 colour3;

namespace lightingOps {
	colour3 loopAllSceneLightsDoLighting(const colour3& inital_colour, const Scene& scene, const Material& material, const Vertex& intersection, const Vector& normal, const Vector& E, BVH* bvh);

	float calcDistanceIntensity(float dist);
	float calcDirectionalLightShadowIntensity (const Vertex& intersection, const Vector& normal, DirectionalLight* light, BVH* bvh, bool pick = false, int rayCount = 1);
	float calcPointLightShadowIntensity (const Vertex& intersection, PointLight* light, BVH* bvh, bool pick = false, int rayCount = 1);
	float calcSpotLightShadowIntensity (const Vertex& intersection, SpotLight* light, BVH* bvh, bool pick = false, int rayCount = 1);

	colour3 calculateAmbient(AmbientLight* light, const colour3& color, const Material& material, const Vertex& intersection, const Vertex& normal, const Vertex& E);
	colour3 calculateDirectionalPhong(DirectionalLight* light, const colour3& color, const Material& material, const Vertex& intersection, const Vertex& normal, const Vertex& E);
	colour3 calculatePointPhong(PointLight* light, const colour3& color, const Material& material, const Vertex& intersection, const Vertex& normal, const Vertex& E);
	colour3 calculateSpotPhong(SpotLight* light, const colour3& color, const Material& material, const Vertex& intersection, const Vertex& normal, const Vertex& E);

	colour3 calcRefraction(const Vector& inc_vec, const Vector& normal, float refraction_index, Vector& refracted_vec);
}