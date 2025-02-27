#version 150

uniform sampler2D noise_tex;        // Texture with 2D noise
uniform sampler1D particles_tex;    // Texture containing fish info
uniform int n_particles;            // Total number of fish
uniform vec2 user_pos;

in float _texcoord;
const float delta_t = 0.00025;      // Time step
const float radius_user = 0.06;     // The radius of the user used for repelling from user
const float radius_repel = 0.025;   // Radius of Gaussian blob used for repelling

float Gaussian(vec2 v,float r2)
{
    return exp(-dot(v,v)/r2);
}

float sqr(float x) {return x*x;}

vec2 gradient_of_Gaussian(vec2 v,float r2)
{
    float d = dot(v,v);
    return d>0 ? -2*v/(sqrt(d)*r2)*exp(-d/r2) : vec2(0);
}

out vec4 fragColor;
void main()
{

    fragColor = vec4(0.5,0.5,1,0);
}
