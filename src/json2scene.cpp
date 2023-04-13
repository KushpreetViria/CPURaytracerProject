// Ray Tracer Assignment JSON->CPP utility
// Winter 2021

#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>
#include <glm/glm.hpp>

#include "schema.h"

#include "json2scene.h"
#include <glm/gtx/transform.hpp>

using json = nlohmann::json;

/****************************************************************************/

static glm::vec3 vector_to_vec3(const std::vector<float>& v) {
	return glm::vec3(v[0], v[1], v[2]);
}

int json_to_scene(json& jscene, Scene& s) {
	json camera = jscene["camera"];
	if (camera.find("field") != camera.end()) {
		s.camera.field = camera["field"];
	}
	if (camera.find("background") != camera.end()) {
		s.camera.background = vector_to_vec3(camera["background"]);
	}

	// Traverse the objects
	json& objects = jscene["objects"];
	for (json::iterator it = objects.begin(); it != objects.end(); ++it) {
		json& object = *it;

		// Every object will have a material, but all parameters are optional
		json& material = object["material"];
		Material m;
		if (material.find("ambient") != material.end()) {
			m.ambient = vector_to_vec3(material["ambient"]);
		}
		if (material.find("diffuse") != material.end()) {
			m.diffuse = vector_to_vec3(material["diffuse"]);
		}
		if (material.find("specular") != material.end()) {
			m.specular = vector_to_vec3(material["specular"]);
		}
		if (material.find("shininess") != material.end()) {
			m.shininess = material["shininess"];
		}
		if (material.find("reflective") != material.end()) {
			m.reflective = vector_to_vec3(material["reflective"]);
		}
		if (material.find("transmissive") != material.end()) {
			m.transmissive = vector_to_vec3(material["transmissive"]);
		}
		if (material.find("refraction") != material.end()) {
			m.refraction = material["refraction"];
		}

		// Every object in the scene will have a type
		if (object["type"] == "sphere") {
			// Every sphere has a position and a radius
			Vertex pos = vector_to_vec3(object["position"]);
			float radius = object["radius"];
			s.objects.push_back(new Sphere(m, radius, pos));
		}
		else if (object["type"] == "cylinder") {
			float radius = object["radius"];
			float height = object["height"];

			std::vector<glm::mat4> transformations;
			getTransformations(object, transformations);

			s.objects.push_back(new Cylinder(m, radius, height, transformations));
		}
		else if (object["type"] == "plane") {
			// Every plane has a position (point of intersection) and a normal
			Vertex pos = vector_to_vec3(object["position"]);
			Vector normal = vector_to_vec3(object["normal"]);
			s.objects.push_back(new Plane(m, pos, normal));
		}
		else if (object["type"] == "mesh") {
			// Every mesh has a list of triangles
			std::vector<Triangle> tris;
			json& ts = object["triangles"];
			for (json::iterator ti = ts.begin(); ti != ts.end(); ++ti) {
				json& t = *ti;
				tris.push_back({ vector_to_vec3(t[0]), vector_to_vec3(t[1]), vector_to_vec3(t[2]) });
			}
			s.objects.push_back(new Mesh(m, tris));
		}
		else {
			std::cout << "*** unrecognized object type " << object["type"] << "\n";
			return -1;
		}
	}

	// Traverse the lights
	json& lights = jscene["lights"];
	for (json::iterator it = lights.begin(); it != lights.end(); ++it) {
		json& light = *it;

		// Every light in the scene will have a colour (ired, igreen, iblue)
		RGB colour = vector_to_vec3(light["color"]);

		if (light["type"] == "ambient") {
			// There should only be one ambient light
			for (Light* l : s.lights) {
				if (l->type == "ambient") {
					std::cout << "*** there should only be one ambient light!\n";
					return -1;
				}
			}
			s.lights.push_back(new AmbientLight(colour));
		}
		else if (light["type"] == "directional") {
			// Every directional light has a direction
			Vector direction = vector_to_vec3(light["direction"]);
			s.lights.push_back(new DirectionalLight(colour, direction));
		}
		else if (light["type"] == "point") {
			// Every point light has a position
			Vertex pos = vector_to_vec3(light["position"]);
			s.lights.push_back(new PointLight(colour, pos));
		}
		else if (light["type"] == "spot") {
			// Every spot light has a position, direction, and cutoff
			Vertex pos = vector_to_vec3(light["position"]);
			Vector direction = vector_to_vec3(light["direction"]);
			float cutoff = light["cutoff"];
			s.lights.push_back(new SpotLight(colour, pos, direction, cutoff));
		}
		else {
			std::cout << "*** unrecognized light type " << light["type"] << "\n";
			return -1;
		}
	}

	return 0;
}

void getTransformations(json object, std::vector<glm::mat4>& transformations) {
	json& ts = object["transformations"];
	for (json::iterator ti = ts.begin(); ti != ts.end(); ++ti) {
		json& t = *ti;
		if (t["type"] == "rotation") {
			transformations.push_back(glm::rotate(glm::radians((float)t["degrees"]), glm::normalize(vector_to_vec3(t["axis"]))));
		}
		else if (t["type"] == "translate") {
			transformations.push_back(glm::translate(vector_to_vec3(t["translate"])));
		}
	}
}


/****************************************************************************/

void printf_rgb(RGB& rgb) {
	printf("RGB( %f, %f, %f )", rgb.r, rgb.g, rgb.b);
}

void printf_vertex(Vertex& v) {
	printf("Vertex( %f, %f, %f )", v.x, v.y, v.z);
}

void printf_vector(Vector& v) {
	printf("Vector( %f, %f, %f )", v.x, v.y, v.z);
}

void printf_material(Material& m) {
	printf("Material( ");
	printf_rgb(m.ambient);
	printf(", ");
	printf_rgb(m.diffuse);
	printf(", ");
	printf_rgb(m.specular);
	printf(", %f, ", m.shininess);
	printf_rgb(m.reflective);
	printf(", ");
	printf_rgb(m.transmissive);
	printf(", %f )", m.refraction);
}
