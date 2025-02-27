#version 410

// Render backfacing part of object. This is to ensure that the ray has an end point
// DO NOT EDIT

uniform mat4 PVM;

in vec3 vertex;
in vec3 normal;
out vec3 ray_terminus;

void main()
{
    gl_Position = PVM * vec4(vertex,1);
    ray_terminus = vertex;
}
