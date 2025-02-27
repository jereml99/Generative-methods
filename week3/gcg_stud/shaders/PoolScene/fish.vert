#version 150
uniform mat4 VM;
uniform mat4 PVM;
uniform mat3 N;

uniform sampler1D particle_tex; // A 1D texture with the fish (or particles since they behave like that)
uniform float particle_no;      // a uniform that serves as index into particle_tex (index to a fish)
uniform float time;

in vec3 vertex;
in vec3 normal;
in vec2 texcoord;

out vec2 _texcoord;
out vec3 _norm; // A varying variable used to transfer the color from
// vertex to fragment program.

float sqr(float x){return x*x;}

void main()
{
    // Compute eyespace normal and position
    _norm = normalize(N*normal);
    _texcoord = texcoord;
    gl_Position = PVM * vec4(vertex,1);
}
