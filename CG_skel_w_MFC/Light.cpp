#include "stdafx.h"
#include "Light.h"

Light::Light(LightType type)
{
	position = vec3();
	direction = vec3(0.0, -1.0, 0.0);
	color = vec3(1.0, 1.0, 1.0);
	ambientIntensity = 1.0;
	diffuseIntensity = 1.0;
	specularIntensity = 1.0;
	lightType = type;
}

void Light::transformWorld(const mat4& transform, bool rotation)
{
	if (lightType == parallel && rotation)
		direction = homo2noHomo(transform * vec4(direction));
	else
		position = homo2noHomo(transform * vec4(position));
}

