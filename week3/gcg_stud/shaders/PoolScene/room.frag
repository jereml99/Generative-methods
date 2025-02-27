#version 150

uniform sampler2D tex;          // Uniform specifying the texture unit
uniform sampler2D hf_tex;       // Height field texture for bump map
uniform vec4 light_pos;

in vec3 pos;
in vec3 u_tangent;
in vec3 v_tangent;
in vec3 norm;
in vec2 _texcoord;

const int MAX_STEP = 100;
const float MAX_HEIGHT = 0.01;
const int IMAGE_DIMS = 1024;

out vec4 fragColor;


void main()
{
    vec3 light_direction = normalize(light_pos.xyz-pos);
    float d = dot(norm, light_direction);
    vec4 color = vec4(0.3)+d*vec4(0.6);
    fragColor = color * texture(tex, _texcoord.xy);
}
