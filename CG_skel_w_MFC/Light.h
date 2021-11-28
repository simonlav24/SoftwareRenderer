#pragma once
#include "vec.h"
#include "mat.h"
//#include "Renderer.h"

struct Material
{
	vec3 color;
	GLfloat ambientCoeficient;
	GLfloat diffuseCoeficient;
	GLfloat specularCoeficient;
	Material() :ambientCoeficient(0.1), diffuseCoeficient(0.5), specularCoeficient(0.1), color(0.8, 0.8, 0.8) {}
};

class Light {
public:
	Light();
	vec3 position;
	vec3 color;

	void transformWorld(const mat4& transform);
};

class AmbientLight : public Light
{
public:
	GLfloat Intensity;
};