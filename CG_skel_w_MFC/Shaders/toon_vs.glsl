
#version 150
#define MAX_NUM_OF_LIGHTS 6
#define PI 3.1415926535897932384626433832795
#define TWOPI 6.2831853071795864769252867665590

#define MAPPING_UV 0
#define MAPPING_CYLINDER 1
#define MAPPING_SPHERE 2
#define MAPPING_PLANAR 3

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
uniform int textureMapping;
uniform bool isNormalMap;

uniform float timeStep;
uniform int isVertexAnimating;

in vec3 vPosition;
in vec3 vNormal;
in vec2 vTexture;
in vec3 vTangent;
in vec3 vBitangent;

out vec4 posInCam;
out vec3 normalInCam;
out vec2 TexCoord;
out mat3 TBN;

void main()
{
    vec3 addition = vec3(0.0f, 0.0f, 0.0f);

    // calc vertex animation
    if (isVertexAnimating == 1)
        addition = vec3(0.0f, cos(vPosition.x + timeStep), sin(vPosition.x + timeStep));
    if (isVertexAnimating == 2)
        addition = vNormal * exp(-pow(- vPosition.x - 5.0 * sin(timeStep), 2.0f));
    vec3 position = vPosition + addition;
    
    vec4 positionInCam = lookAt * worldModelMat * vec4(position, 1.0);
    gl_Position = proj * positionInCam;
    posInCam = positionInCam;
    normalInCam = normalize(lookAt * normalMat * vec4(vNormal, 0.0)).xyz;

    // texture mapping
    if(textureMapping == MAPPING_UV)
    {
        TexCoord = vTexture;
    }
    else if(textureMapping == MAPPING_CYLINDER)
    {
        float theta = (atan(vPosition.z, vPosition.x) + PI) / TWOPI;
        float h = vPosition.y;
        TexCoord = vec2(theta, h);
    }
    else if(textureMapping == MAPPING_PLANAR)
    {
        TexCoord = vec2(vPosition.x, vPosition.y);
    }
    else if(textureMapping == MAPPING_SPHERE)
    {
        float r = sqrt(vPosition.x * vPosition.x + vPosition.y * vPosition.y + vPosition.z * vPosition.z);
        float theta = (atan(vPosition.z, vPosition.x) + PI) / TWOPI;
        float phi = 1.0f - acos(vPosition.y / r) / PI;
        TexCoord = vec2(theta, phi);
    }
    
    if(isNormalMap)
	{
        vec3 tanInCam = normalize(lookAt * normalMat * vec4(vTangent, 0)).xyz;
	    vec3 bitanInCam = normalize(lookAt * normalMat * vec4(vBitangent, 0) ).xyz;
        tanInCam = normalize(tanInCam - normalInCam * dot(normalInCam, tanInCam));
		bitanInCam = normalize(bitanInCam - normalInCam * dot(normalInCam, bitanInCam));
		TBN = mat3(tanInCam, bitanInCam, normalInCam);
	}
}
