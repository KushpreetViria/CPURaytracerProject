#pragma once
#include <glm/glm.hpp>
#include "schema.h"

typedef glm::vec3 point3;
typedef glm::vec3 colour3;

extern double fov;
extern colour3 background_colour;

void choose_scene(char const *fn);
bool trace(const point3 &e, const point3 &s, colour3 &colour, Object*& objectHit, bool pick);
bool SingleTrace(const point3& e, const point3& s, colour3& colour, Object*& objectHit, bool pick);
