#version 150

uniform sampler2D ref_tex;          // Reflection texture
uniform sampler2D hf_tex;           // Height field texture
uniform sampler2D bg_tex;           // Background texture
uniform vec2 win_dim;               // Windows dimensions

in vec3 pos;
in vec3 norm;
in vec2 _texcoord;
in vec3 u_tangent;
in vec3 v_tangent;

float sqr(float x){return x*x;}

const float n_air = 1.000293;       // Refractive index of air
const float n_water = 1.3330;       // Refractive index of water


out vec4 fragColor;
void main()
{
    float F = 0.5;
    vec2 tc = gl_FragCoord.xy/win_dim;
    fragColor =  F*texture(ref_tex, tc)+(1.0-F)*(0.3*vec4(0.5,0.8,0.7,0)+0.7*texture(bg_tex, tc));
}
