#version 150
uniform sampler2D tex;  // Uniform specifying the texture unit
in vec3 _norm;
in vec2 _texcoord;

uniform vec4 light_pos;
uniform vec3 light_half_vec;
uniform vec4 light_diff;
uniform vec4 light_spec;
uniform vec4 light_amb;

uniform vec4 mat_diff;
uniform vec4 mat_spec;
uniform float mat_spec_exp;

out vec4 fragColor;
void main()
{
    vec4 base_color = texture(tex, _texcoord)*mat_diff;
    vec4 color = base_color*0.5;

    fragColor = color;
}
