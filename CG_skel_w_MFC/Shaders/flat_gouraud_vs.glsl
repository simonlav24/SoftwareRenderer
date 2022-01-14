
#version 150
#define MAX_NUM_OF_LIGHTS 6
#define LIGHT_POINT 0
#define LIGHT_PARALLEL 1
#define LIGHT_AMBIENT 2

#define PI 3.1415926535897932384626433832795
#define TWOPI 6.2831853071795864769252867665590

#define MAPPING_UV 0
#define MAPPING_CYLINDER 1
#define MAPPING_SPHERE 2
#define MAPPING_PLANAR 3

vec3 calculateAmbientLight();
vec3 calculateDiffusionLight(in vec4 position, in vec4 normal);
vec3 calculateSpecularLight(in vec4 position, in vec4 normal);
vec2 calculateEnvironment(in vec4 position, in vec4 normal);

in vec3 vPosition;
in vec3 vNormal;
in vec3 vCenter;
in vec2 vTexture;
out vec2 vTextureCoordinates;
out vec4 color;
out vec3 totalColorOut;
out vec2 environmentCoords;

uniform mat4 lookAt;
uniform mat4 proj;
uniform mat4 worldModelMat;
uniform mat4 normalMat;

uniform vec3 matAmbient;
uniform vec3 matDiffuse;
uniform vec3 matSpecular;
uniform	vec3 matEmissive;
uniform float matShininess;

uniform float timeStep;
uniform bool isVertexAnimating;

uniform vec3 viewerPos;
uniform int textureMapping;
uniform bool isEnvironment;

// point lights
uniform vec3 lightPositions[MAX_NUM_OF_LIGHTS];
uniform vec3 lightColors[MAX_NUM_OF_LIGHTS];
uniform int lightTypes[MAX_NUM_OF_LIGHTS];
uniform int lightCount;

void main()
{
    vec3 addition = vec3(0.0f, 0.0f, 0.0f);
    if(isVertexAnimating)
        addition = vec3(0.0f, cos(vPosition.x + timeStep), sin(vPosition.x + timeStep));
    vec4 pos = vec4(vPosition + addition, 1.0);
    vec4 positionInCam = lookAt * worldModelMat * pos;
    vec4 NormalInCam = lookAt * normalMat * vec4(vNormal, 0.0);
    
    gl_Position = proj * positionInCam;

    vec3 totalColor = vec3(0.0, 0.0, 0.0);
    totalColor += calculateAmbientLight();
    totalColor += calculateDiffusionLight(positionInCam, NormalInCam);
    totalColor += calculateSpecularLight(positionInCam, NormalInCam);
    totalColor += matEmissive;

    totalColorOut = totalColor;
    
    color = vec4(totalColor, 1.0);

    // texture mapping
    if(textureMapping == MAPPING_UV)
    {
        vTextureCoordinates = vTexture;
    }
    else if(textureMapping == MAPPING_CYLINDER)
    {
        float theta = (atan(vPosition.z, vPosition.x) + PI) / TWOPI;
        float h = vPosition.y;
        vTextureCoordinates = vec2(theta, h);
    }
    else if(textureMapping == MAPPING_PLANAR)
    {
        vTextureCoordinates = vec2(vPosition.x, vPosition.y);
    }
    else if(textureMapping == MAPPING_SPHERE)
    {
        float r = sqrt(vPosition.x * vPosition.x + vPosition.y * vPosition.y + vPosition.z * vPosition.z);
        float theta = (atan(vPosition.z, vPosition.x) + PI) / TWOPI;
        float phi = 1.0f - acos(vPosition.y / r) / PI;
        vTextureCoordinates = vec2(theta, phi);
    }

    if(isEnvironment)
    {
        environmentCoords = calculateEnvironment(positionInCam, NormalInCam);
    }

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

vec2 calculateEnvironment(in vec4 position, in vec4 normal)
{
    vec3 normalInCam = normal.xyz;

    vec3 dirToViewer = (lookAt * vec4(viewerPos, 1.0)).xyz - position.xyz;
    vec3 reflected = dirToViewer - 2.0 * dot(dirToViewer, normalInCam) * normalInCam;
    reflected = -normalize(reflected); 

    float r = sqrt(reflected.x * reflected.x + reflected.y * reflected.y + reflected.z * reflected.z);
    float theta = (atan(reflected.z, reflected.x) + PI) / TWOPI;
    float phi = 1.0f - acos(reflected.y / r) / PI;
    vec2 sphericCoord = vec2(theta, phi);

    return sphericCoord;
    //return texture(environmentTexture, sphericCoord).xyz;
}