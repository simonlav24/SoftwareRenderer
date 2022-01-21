
#version 150

in vec3 vPosition;
in vec3 vColor;
out vec4 color;

uniform mat4 lookAt;
uniform mat4 proj;
uniform mat4 worldModelMat;
uniform vec3 wireColor;

uniform float timeStep;
uniform int isVertexAnimating;
 
void main()
{
    color = vec4(wireColor, 1.0);
    vec3 addition = vec3(0.0f, 0.0f, 0.0f);
    if(isVertexAnimating == 1)
        addition = vec3(0.0f, cos(vPosition.x + timeStep), sin(vPosition.x + timeStep));
    vec4 pos = vec4(vPosition + addition, 1.0);
    gl_Position = proj * lookAt * worldModelMat * pos;
}
