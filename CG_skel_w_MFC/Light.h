#pragma once
#include "vec.h"
#include "mat.h"
#include "Renderer.h"


class Light {
public:
	Light();
	vec3 position;
	vec3 color;

	void transformWorld(const mat4& transform);

};