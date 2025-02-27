#version 410

uniform mat4 VM;
uniform mat4 PVM;

uniform float time;
uniform float delta_time;

in vec3 vertex;
in vec4 velocity;

out vec4 pos;
out vec4 pos_old;

void main()
{
        float t0 = velocity.w;
        float t = mod(time-t0,0.4);
        float t_old = t - delta_time;

        gl_Position = PVM * vec4(vertex, 1.0);
        pos = gl_Position;
        pos_old = gl_Position;
}
