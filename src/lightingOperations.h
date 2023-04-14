#pragma once

#include <glm/glm.hpp>
#include "schema.h"

typedef glm::vec3 point3;
typedef glm::vec3 colour3;

namespace lightingOps {
	float calcDistanceIntensity(float dist);
	float calcDirectionalLightShadowIntensity (const Vertex& intersection, const Vector& normal, DirectionalLight* light, const Scene& scene, int rayCount = 1);
	float calcPointLightShadowIntensity (const Vertex& intersection, PointLight* light, const Scene& scene, int rayCount = 1);
	float calcSpotLightShadowIntensity (const Vertex& intersection, SpotLight* light, const Scene& scene, int rayCount = 1);

	colour3 calculateAmbient(AmbientLight* light, const colour3& color, Material* material, const Vertex& intersection, const Vertex& normal, const Vertex& E);
	colour3 calculateDirectionalPhong(DirectionalLight* light, const colour3& color, Material* material, const Vertex& intersection, const Vertex& normal, const Vertex& E);
	colour3 calculatePointPhong(PointLight* light, const colour3& color, Material* material, const Vertex& intersection, const Vertex& normal, const Vertex& E);
	colour3 calculateSpotPhong(SpotLight* light, const colour3& color, Material* material, const Vertex& intersection, const Vertex& normal, const Vertex& E);

	colour3 calcRefraction(const Vector& inc_vec, const Vector& normal, float refraction_index, Vector& refracted_vec);
}