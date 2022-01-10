
#version 150 
#define MAX_NUM_OF_LIGHTS 6
#define LIGHT_POINT 0
#define LIGHT_PARALLEL 1
#define LIGHT_AMBIENT 2

vec3 calculateAmbientLight();
vec3 calculateDiffusionLight(in vec4 position, in vec4 normal);
vec3 calculateSpecularLight(in vec4 position, in vec4 normal);

uniform mat4 lookAt;
uniform mat4 proj;
uniform mat4 worldModelMat;
uniform mat4 normalMat;

uniform vec3 matAmbient;
uniform vec3 matDiffuse;
uniform vec3 matSpecular;
uniform	vec3 matEmissive;
uniform float matShininess;

uniform vec3 viewerPos;
uniform bool isTexturized;

// point lights
uniform vec3 lightPositions[MAX_NUM_OF_LIGHTS];
uniform vec3 lightColors[MAX_NUM_OF_LIGHTS];
uniform int lightTypes[MAX_NUM_OF_LIGHTS];
uniform int lightCount;

in vec4 posInCam;
in vec3 normalInCam;
in vec2 TexCoord;

out vec4 fcolor;
uniform sampler2D ourTexture;

void main()
{
	vec3 totalColor = vec3(0.0, 0.0, 0.0);
    totalColor += calculateAmbientLight();
    totalColor += calculateDiffusionLight(posInCam, vec4(normalInCam, 0.0));
    totalColor += calculateSpecularLight(posInCam, vec4(normalInCam, 0.0));
    totalColor += matEmissive;
    
    if(isTexturized)
        totalColor *= texture(ourTexture, TexCoord).xyz;

    fcolor = vec4(totalColor, 1);
}

vec3 calculateAmbientLight()
{
    vec3 totalAmbientLight = vec3(0.0, 0.0, 0.0);
    for(int i = 0; i < lightCount; i++)
    {
        totalAmbientLight += lightColors[i] * matAmbient;
    }
    return totalAmbientLight;
}

vec3 calculateDiffusionLight(in vec4 position, in vec4 normal)
{
    vec3 pos = position.xyz;
    vec3 normalInCam = normal.xyz;

    vec3 totalDiffusionLight = vec3(0.0, 0.0, 0.0);
    for(int i = 0; i < lightCount; i++)
    {
        vec3 diffusionAmount = vec3(0.0, 0.0, 0.0);
        vec3 dirFromLight;
        if(lightTypes[i] == LIGHT_AMBIENT)
            continue;
        if(lightTypes[i] == LIGHT_POINT)
            dirFromLight = normalize((lookAt * vec4(lightPositions[i], 1.0)).xyz - pos);
        if(lightTypes[i] == LIGHT_PARALLEL)
            dirFromLight = -normalize((lookAt * vec4(lightPositions[i], 0.0)).xyz);
        float dotProd = max(0.0, dot(normalize(normalInCam), normalize(dirFromLight)));
        diffusionAmount = lightColors[i] * matDiffuse * dotProd;
        totalDiffusionLight += diffusionAmount;
        
    }
    return totalDiffusionLight;
}

vec3 calculateSpecularLight(in vec4 position, in vec4 normal)
{
    vec3 pos = position.xyz;
    vec3 normalInCam = normal.xyz;
    vec3 reflected;

    vec3 totalSpecularLight = vec3(0.0, 0.0, 0.0);
    for(int i = 0; i < lightCount; i++)
    {
        vec3 specularAmount = vec3(0.0, 0.0, 0.0);
        vec3 dirFromLight;
        if(lightTypes[i] == LIGHT_AMBIENT)
            continue;
        if(lightTypes[i] == LIGHT_POINT)
            dirFromLight = -normalize((lookAt * vec4(lightPositions[i], 1.0)).xyz - pos);
        if(lightTypes[i] == LIGHT_PARALLEL)
            dirFromLight = normalize((lookAt * vec4(lightPositions[i], 0.0)).xyz);
        vec3 reflected = dirFromLight - 2.0 * dot(dirFromLight, normalInCam) * normalInCam;
        vec3 dirToViewer = (lookAt * vec4(viewerPos, 1.0)).xyz - pos;
        float dotProd = dot(normalize(reflected), normalize(dirToViewer));

        if (dotProd <= 0)
			continue;

		dotProd = pow(dotProd, matShininess);
        specularAmount = lightColors[i] * matSpecular * dotProd;
        totalSpecularLight += specularAmount;
    }
    return totalSpecularLight;
}