
#version 150
#define MAX_NUM_OF_LIGHTS 6

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
    TexCoord = vTexture;

    normalInCam = normalize(lookAt * normalMat * vec4(vNormal, 0.0)).xyz;
}
