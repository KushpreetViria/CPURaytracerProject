#pragma once

#include <glm/glm.hpp>
#include <iostream>

#include "schema.h"

typedef glm::vec3 point3;
typedef glm::vec3 colour3;

/// <summary>
/// Find if this ray intersects with any objects in the scene and returns any relavant information
/// </summary>
/// <param name="e">= Ray origin</param>
/// <param name="d">= Ray directional vector</param>
/// <param name="scene">= Scene with objects</param>
/// <param name="minT"> = t values below this will not be considered intersections</param>
/// <returns>
/// A tuple containing the following values from index 0 to 3: 
/// float t value, 
/// Object* intersecting object, 
/// point3 intersection normal, 
/// point3 intersection point.
/// </returns>
std::tuple<float, Object*, point3, point3> rayIntersectScene(point3 e, point3 d, Scene scene, float minT);

namespace cylinderOps {
    struct CylinderIntersectResult
    {
        point3 intersection;
        point3 normal;
        float t;

        CylinderIntersectResult() : t(-1)
        {}
    };

    bool rayIntersects(const point3& e, const point3& d, Cylinder* cylinder, CylinderIntersectResult& result, float minT);
}

namespace sphereOps {
    struct SphereIntersectResult
    {
        point3 intersection_near;
        point3 normal_near;
        float t_near;

        SphereIntersectResult() : t_near(-1)
        {}
    };

    bool rayIntersects(const point3& e, const point3& d, Sphere* sphere, SphereIntersectResult& result, float minT);
}

namespace planeOps {
    struct PlaneIntersectResult
    {
        point3 intersection;
        point3 normal;
        float t;

        PlaneIntersectResult() : t(-1)
        {}
    };

    bool rayIntersects(const point3& e, const point3& d, Plane* plane, PlaneIntersectResult& result, float minT);
    bool rayIntersects(const point3& e, const point3& d, point3 n, point3 a, PlaneIntersectResult& result, float minT);
}


namespace meshOps {
    struct MeshRayIntersectResult
    {
        point3 intersection;
        point3 normal;
        float t;

        MeshRayIntersectResult() : t(-1)
        {}
    };

	bool rayIntersects(const point3& e, const point3& d, Mesh* mesh, MeshRayIntersectResult& result, float minT);
}