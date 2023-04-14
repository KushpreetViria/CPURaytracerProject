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
/// Vector intersection normal, 
/// Vertex intersection point.
/// </returns>
std::tuple<float, Object*, Vector, Vertex> rayIntersectScene(const Vertex& e, const Vector& d, const Scene& scene, float minT);

namespace cylinderOps {
    struct CylinderIntersectResult
    {
        Vertex intersection;
        Vector normal;
        float t;

        CylinderIntersectResult() : t(-1)
        {}
    };

    bool rayIntersects(const Vertex& e, const Vector& d, Cylinder* cylinder, CylinderIntersectResult& result, float minT);
}

namespace sphereOps {
    struct SphereIntersectResult
    {
        Vertex intersection_near;
        Vector normal_near;
        float t_near;

        SphereIntersectResult() : t_near(-1)
        {}
    };

    bool rayIntersects(const Vertex& e, const Vector& d, Sphere* sphere, SphereIntersectResult& result, float minT);
}

namespace planeOps {
    struct PlaneIntersectResult
    {
        Vertex intersection;
        Vector normal;
        float t;

        PlaneIntersectResult() : t(-1)
        {}
    };

    bool rayIntersects(const Vertex& e, const Vector& d, Plane* plane, PlaneIntersectResult& result, float minT);
    bool rayIntersects(const Vertex& e, const Vector& d, const Vector& n, const Vertex& a, PlaneIntersectResult& result, float minT);
}


namespace meshOps {
    struct MeshRayIntersectResult
    {
        Vertex intersection;
        Vector normal;
        float t;

        MeshRayIntersectResult() : t(-1)
        {}
    };

	bool rayIntersects(const Vertex& e, const Vector& d, Mesh* mesh, MeshRayIntersectResult& result, float minT);
}