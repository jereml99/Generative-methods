#version 410

// Vertex shader for rendering object. This shader outputs start point
// for ray and direction and also the surface normal
// DO NOT EDIT

in vec3 vertex;
in vec3 normal;

out vec3 ray_dir;
out vec3 ray_origin;
out vec3 norm;

// The projection, view model matrix PVM and the normal matrix N
uniform mat4 PVM;
uniform vec4 eye_pos;

void main()
{
    gl_Position = PVM * vec4(vertex,1);
    norm = normal;
    ray_origin = vertex;
    ray_dir =  (ray_origin - eye_pos.xyz);
}
