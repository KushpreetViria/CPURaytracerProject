#pragma once

#include <glm/glm.hpp>
#include "schema.h"

typedef glm::vec3 point3;
typedef glm::vec3 colour3;

float calcDistanceIntensity(float dist);
float calcDirectionalLightShadowIntensity(point3 intersection, point3 normal, DirectionalLight* light, Scene scene, int rayCount=1);
float calcPointLightShadowIntensity(point3 intersection, PointLight* light, Scene scene, int rayCount = 1);
float calcSpotLightShadowIntensity(point3 intersection, SpotLight* light, Scene scene, int rayCount = 1);

point3 calculateAmbient(AmbientLight* light, point3 color, Material* material, point3 intersection, point3 normal, point3 E);
point3 calculateDirectionalPhong(DirectionalLight* light, point3 color, Material* material, point3 intersection, point3 normal, point3 E);
point3 calculatePointPhong(PointLight* light, point3 color, Material* material, point3 intersection, point3 normal, point3 E);
point3 calculateSpotPhong(SpotLight* light, point3 color, Material* material, point3 intersection, point3 normal, point3 E);

point3 calcRefraction(const point3 inc_vec, point3 normal, float refraction_index, point3& refracted_vec);