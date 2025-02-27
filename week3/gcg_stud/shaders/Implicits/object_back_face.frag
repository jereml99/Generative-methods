#version 410

// Render backfacing part of object. This is to ensure that the ray has an end point
// DO NOT EDIT

uniform vec4 eye_pos;
uniform vec3 y_dir;

in vec3 ray_terminus;
out vec4 fragColor;     // Output color


void main()
{
    // Output end point for the ray    
    fragColor.rgb = ray_terminus;
}

