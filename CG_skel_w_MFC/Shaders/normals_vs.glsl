
#version 150



uniform mat4 worldModel;
uniform mat4 normalMat;
uniform mat4 proj;
uniform mat4 lookAt;

in vec3 vPosition;
in vec3 vNormal;
in vec3 vColor;
out vec4 color;

void main()
{
    color = vec4(vColor, 1);

    vec4 origin = vec4(vPosition, 1.0);
    vec4 normal = vec4(vNormal, 1.0);
    origin = worldModel * origin;
    normal = normalMat * normal;
    normal.w = 0.0;
    normal = normalize(normal);

    vec4 point = origin + normal;

    origin = proj * lookAt * origin;
    point = proj * lookAt * point;

    gl_Position = point;
}
