#include "stdafx.h"
#include "Light.h"

Light::Light()
{
	position = vec3();
	color = vec3(1.0, 1.0, 1.0);
}

void Light::transformWorld(const mat4& transform)
{
	position = homo2noHomo(transform * vec4(position));
}

