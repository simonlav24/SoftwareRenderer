
#version 150

in vec3 vPosition;
in vec3 vColor;
out vec4 color;

uniform mat4 lookAt;
uniform mat4 proj;
uniform mat4 worldModelMat;
 
void main()
{
    color = vec4(vColor, 1.0);
    vec4 pos = vec4(vPosition, 1.0);
    gl_Position = proj * lookAt * worldModelMat * pos;
}