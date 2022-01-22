#pragma once
#include "vec.h"
#include "mat.h"

#define MAPPING_UV 0
#define MAPPING_CYLINDER 1
#define MAPPING_SPHERE 2
#define MAPPING_PLANAR 3

#define LOAD_TEX_COLOR 0
#define LOAD_TEX_ENVIRONMENT 1
#define LOAD_TEX_NORMAL 2

enum LightType {point, parallel, ambience};

struct Texture
{
	int width;
	int height;
	int nrChannels;
	GLuint textureId;
	unsigned char* data;
};

struct Material
{
	vec3 color;
	vec3 ambientColor;
	vec3 diffuseColor;
	vec3 specularColor;
	vec3 emissiveColor;
	GLfloat shininessCoeficient;

	bool isTexturized;
	Texture textureImage;
	int textureMappingMode;

	bool isEnvironment;
	Texture textureEnvironment;
	float environmentStrength;

	bool isNormalMap;
	Texture textureNormal;
	float normalStrength;

	bool isNoiseTexture;
	Texture textureNoise;

	Material() :
		color(0.8f, 0.8f, 0.8f),
		ambientColor(0.1f, 0.1f, 0.1f),
		diffuseColor(0.4f, 0.5f, 0.5f),
		specularColor(0.8f, 0.8f, 0.8f),
		emissiveColor(0.0f, 0.0f, 0.0f),
		shininessCoeficient(16.0f),
		isTexturized(false),
		textureMappingMode(MAPPING_UV),
		isEnvironment(false),
		environmentStrength(0.5f),
		isNormalMap(false),
		normalStrength(1.0f),
		isNoiseTexture(false)
	{}
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