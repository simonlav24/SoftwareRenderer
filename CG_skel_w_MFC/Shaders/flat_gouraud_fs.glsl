
#version 150 

in vec4 color;
in vec2 vTexture;
in vec2 vTextureCoordinates;

uniform bool isTexturized;
uniform bool isEnvironment;
uniform float environmentStrength;

out vec4 fcolor;
in vec3 totalColorOut;
in vec2 environmentCoords;

uniform sampler2D materialTexture;
uniform sampler2D environmentTexture;

void main()
{
	vec3 totalColor = totalColorOut;
	if (isTexturized)
	{
		totalColor = totalColor * texture(materialTexture, vTextureCoordinates).xyz;
	}

	if (isEnvironment)
	{
		vec3 environmentColor = texture(environmentTexture, environmentCoords).xyz;
		totalColor = environmentStrength * environmentColor + (1.0f - environmentStrength) * totalColor;
	}

	fcolor = vec4(totalColor, 1.0f);
}
