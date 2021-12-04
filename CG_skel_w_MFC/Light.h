#pragma once
#include "vec.h"
#include "mat.h"
//#include "Renderer.h"

enum LightType {point, parallel, ambience};

struct Material
{
	bool special;
	vec3 color;
	GLfloat shininessCoeficient;
	vec3 ambientColor;
	vec3 diffuseColor;
	vec3 specularColor;
	Material() :
		color(0.8, 0.8, 0.8), ambientColor(0.1, 0.1, 0.1), diffuseColor(0.5, 0.5, 0.5), specularColor(0.8, 0.8, 0.8), shininessCoeficient(6.0){}
};

class Light {
public:
	Light(LightType type=point);
	vec3 position;
	vec3 direction;
	vec3 color;
	LightType lightType;
	GLfloat ambientIntensity;
	GLfloat diffuseIntensity;
	GLfloat specularIntensity;
	void transformWorld(const mat4& transform, bool rotation=false);
};

class AmbientLight : public Light
{
public:
	GLfloat Intensity;
};