
#version 150 

in vec4 color;
in vec2 vTexture;
in vec2 vTextureCoordinates;
uniform bool isTexturized;

out vec4 fcolor;
in vec3 totalColorOut;

uniform sampler2D ourTexture;

void main()
{
	if (isTexturized)
	{
		fcolor = vec4(totalColorOut * texture(ourTexture, vTextureCoordinates).xyz, 1.0f);
	}
	else
	{
		fcolor = color;
	}
}
