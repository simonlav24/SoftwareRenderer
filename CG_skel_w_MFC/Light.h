#pragma once
#include "vec.h"
#include "mat.h"
//#include "Renderer.h"

struct Material
{
	vec3 color;
	//GLfloat ambientCoeficient;
	//GLfloat diffuseCoeficient;
	//GLfloat specularCoeficient;
	GLfloat shininessCoeficient;
	vec3 ambientColor;
	vec3 diffuseColor;
	vec3 specularColor;
	Material() :/*ambientCoeficient(0.1), diffuseCoeficient(0.5), specularCoeficient(0.5),*/
		color(0.8, 0.8, 0.8), ambientColor(0.1, 0.1, 0.1), diffuseColor(0.5, 0.5, 0.5), specularColor(0.8, 0.8, 0.8), shininessCoeficient(6.0){}
};

class Light {
public:
	Light();
	vec3 position;
	vec3 color;
	GLfloat ambientIntensity;
	GLfloat diffuseIntensity;
	GLfloat specularIntensity;
	void transformWorld(const mat4& transform);
};

class AmbientLight : public Light
{
public:
	GLfloat Intensity;
};