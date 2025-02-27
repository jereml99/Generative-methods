#version 410
layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

uniform mat4 P;

in vec4 pos_old[1];
in vec4 pos[1];

void main(void)
{

	vec4 p_o_1 = pos_old[0];
	vec4 p_o_2 = pos_old[0];

	vec4 p_1 = pos[0];
	vec4 p_2 = pos[0];

	gl_Position = P * p_2;
        EmitVertex();
        gl_Position = P * p_1;
        EmitVertex();
        gl_Position = P * p_o_2;
        EmitVertex();
        gl_Position = P * p_o_1;
        EmitVertex();
}

