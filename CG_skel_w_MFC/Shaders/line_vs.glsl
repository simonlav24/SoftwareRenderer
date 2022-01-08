
#version 150

in vec4 vPosition;
in vec3 vColor;
out vec4 color;

uniform mat4 transform;
 
void main()
{
    color = vec4(vColor, 1);
    gl_Position = transform * vPosition;
}
