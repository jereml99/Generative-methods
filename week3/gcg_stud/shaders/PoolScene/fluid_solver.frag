#version 150
in vec2 _texcoord;
uniform sampler2D h_sigma_tex;

const float d = 0.998;            // Damping
const float delta_t = 0.0125;     // Time step
const float delta_s = 0.0009765;  // Spatial discretization
const float c = 0.02;             // Wave speed constant

// Extract water column height from texture
float h(vec2 tc) {return texture(h_sigma_tex, tc).x;}

out vec4 fragColor;
void main()
{
    fragColor = vec4(0.0);

}
