
#version 150
#define MAX_NUM_OF_LIGHTS 6
#define PI 3.1415926535897932384626433832795
#define TWOPI 6.2831853071795864769252867665590

#define MAPPING_UV 0
#define MAPPING_CYLINDER 1
#define MAPPING_PLANNAR 2

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

in vec3 vPosition;
in vec3 vNormal;
in vec2 vTexture;

out vec4 posInCam;
out vec3 normalInCam;
out vec2 TexCoord;

void main()
{
    vec4 positionInCam = lookAt * worldModelMat * vec4(vPosition, 1.0);

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
    else if(textureMapping == MAPPING_PLANNAR)
    {
        TexCoord = vec2(vPosition.x, vPosition.y);
    }

    
}
