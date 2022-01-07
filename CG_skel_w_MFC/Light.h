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
	vec3 emissiveColor;
	Material() :
		color(0.8f, 0.8f, 0.8f), ambientColor(0.1f, 0.1f, 0.1f), diffuseColor(0.5f, 0.5f, 0.6f), specularColor(0.8f, 0.8f, 0.8f),
		emissiveColor(0.0f, 0.0f, 0.0f), shininessCoeficient(6.0f){}
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