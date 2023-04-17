#pragma once

#include <iostream>
#include <glm/gtx/string_cast.hpp>

#include "../schema.h"

class BVHBoundingBox {
public:
	BVHBoundingBox(float _x_min, float _y_min, float _z_min, float _x_max, float _y_max, float _z_max)
		: x_min(_x_min), y_min(_y_min), z_min(_z_min), x_max(_x_max), y_max(_y_max), z_max(_z_max) {}

	BVHBoundingBox(Vector _sortAxis, std::vector<Object*> _objs, float _x_min, float _y_min, float _z_min, float _x_max, float _y_max, float _z_max)
		: sortAxis(_sortAxis), myObjects(_objs), x_min(_x_min), y_min(_y_min), z_min(_z_min), x_max(_x_max), y_max(_y_max), z_max(_z_max) {}

	float get_x_min() const { return x_min; }
	float get_y_min() const { return y_min; }
	float get_z_min() const { return z_min; }
	float get_x_max() const { return x_max; }
	float get_y_max() const { return y_max; }
	float get_z_max() const { return z_max; }
	
	Vertex get_center() const { return Vertex(x_min + x_max / 2.0f, y_min + y_max / 2.0f, z_min + z_max / 2.0f); }
	Vector get_sort_Axis() const { return sortAxis; }

	float get_width() const { return x_max - x_min; }
	float get_height() const { return y_max - y_min; }
	float get_depth() const { return z_max - z_min; }
	std::vector<Object*> get_objects() const { return myObjects; }

	void set_Axis(Vector axis) { this->sortAxis = axis; }

	std::string toString() {
		char buff[1024];
		snprintf(buff, sizeof(buff), "Min: %f, %f, %f, Max:%f, %f, %f", x_min, y_min, z_min, x_max, y_max, z_max);
		std::string buffAsStdStr = buff;
		return buffAsStdStr;
	}

	static BVHBoundingBox* constructFromObject(std::vector<Object*> objects, Vector BVHSortingAxis);

	static float getBoundingBoxOverlapPercentage(const BVHBoundingBox& a, const BVHBoundingBox& b);

	~BVHBoundingBox();

private:
	float x_min;
	float y_min;
	float z_min;
	float x_max;
	float y_max;
	float z_max;

	Vector sortAxis;
	std::vector<Object*> myObjects;
};