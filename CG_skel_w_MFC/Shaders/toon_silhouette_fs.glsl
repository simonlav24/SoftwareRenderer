
#version 150 
#define MAX_NUM_OF_LIGHTS 6
#define LIGHT_POINT 0
#define LIGHT_PARALLEL 1
#define LIGHT_AMBIENT 2

#define PI 3.1415926535897932384626433832795
#define TWOPI 6.2831853071795864769252867665590

vec3 calculateAmbientLight();
vec3 calculateDiffusionLight(in vec4 position, in vec4 normal);
vec3 calculateSpecularLight(in vec4 position, in vec4 normal);

vec3 calculateEnvironment(in vec4 position, in vec4 normal);

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
uniform bool isEnvironment;
uniform bool isNormalMap;
uniform float environmentStrength;

// point lights
uniform vec3 lightPositions[MAX_NUM_OF_LIGHTS];
uniform vec3 lightColors[MAX_NUM_OF_LIGHTS];
uniform int lightTypes[MAX_NUM_OF_LIGHTS];
uniform int lightCount;

in vec4 posInCam;
in vec3 normalInCam;
in vec2 TexCoord;

in vec3 vPosition;
in vec3 vNormal;

in mat3 TBN;

out vec4 fcolor;
uniform sampler2D materialTexture;
uniform sampler2D environmentTexture;
uniform sampler2D normalMapTexture;

void main()
{
    
    // calculate toon color
    vec4 totalColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);

    fcolor = totalColor;
}