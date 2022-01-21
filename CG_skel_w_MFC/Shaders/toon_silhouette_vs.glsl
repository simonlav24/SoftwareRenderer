
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
uniform bool isVertexAnimating;

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
    if(isVertexAnimating)
        addition = vec3(0.0f, cos(vPosition.x + timeStep), sin(vPosition.x + timeStep));
    vec4 pos = vec4(vPosition + addition, 1.0);
    gl_Position = proj * lookAt * worldModelMat * pos;
}
