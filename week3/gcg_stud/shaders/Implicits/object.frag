#version 410

// ------------------------------------------------------------------------------------------
// An almost fully functional ray tracer for implicit surfaces. Presently, it renders only a 
// single sphere and it takes constant length steps. Your tasks are to 
// 1. Compute the gradient correctly
// 2. make it more efficient by taking adaptive steps.
// 3. Change the sphere to an ellipsoid.
// 4. Make a composition by blending at least three implicits.
// 5. Draw a periodic structure (e.g. gyroid or grid of spheres)
// 6. NON-MANDATORY: Add shadows, ambient occlusion, reflections or other rendering features.
// ------------------------------------------------------------------------------------------


uniform sampler2DRect gtex;
uniform float T;        // Time
uniform vec4 eye_pos; 
uniform vec3 y_dir;     // What direction considered up

in vec3 ray_dir;        // Direction of ray
in vec3 ray_origin;     // Origin of ray: where the ray intersects the bounding box
in vec3 norm;           // Normal at ray origin
out vec4 fragColor;     // Output color

const float thresh = 1e-4;                     // Required surface proximity.
const float inv_thresh_times_2 = 2.0/thresh;    // Used for central differences

// For distance fields, Lipschitz const should be 1:
const float lipschitz_const = 1.0;

// Compute distance. Right now this function computes the distance to a single
// sphere
float dist(vec3 p) {
    vec3 c = vec3(0.0, -0.1, 0.0);
    float r = 0.5;
    return length(p-c) - r;
}

// Compute the gradient. For many implcits, we can easily compute the 
// analytic derivatives, but this is a general solution that does not
// require us to find the derivative for every new implicit.
vec3 dist_grad(vec3 p) {
    return vec3(0,1,0);
}


vec3 shade(vec3 p, vec3 n, float t) {
    // This function computes the shading as a sum of three terms:
    // ambient, diffuse, and specular. There are diffuse and specular
    // contributions for two lights - a skylight above and a light source
    // in the eye. The amount of specular reflection is computed via a
    // smooth step function. Note that specular is just a highlight.
    // It would not be hard to add secondary rays and use these to compute
    // specular reflections and cast shadows, but it would reduce performance.
    
    // Color is initialized to the ambient color
    vec3 color = vec3(0.05,0.05, 0.15);
    // Add diffuse and specular contributions for light source in the eye
    float d = dot(n,normalize(eye_pos.rgb));
    color +=  vec3(0.5,0.5,0.75) * d;
    color +=  vec3(0.1,0.1,0.1) * smoothstep(0.75,0.95,d);
    d = dot(n, y_dir);
    // Add diffuse and specular contribution for skylight
    color +=  vec3(0.25,0.25,0.2) * d;
    color +=  vec3(0.05,0.05,0.04) * smoothstep(0.4,0.6,d);
    return color;//*clamp(1.0-1.0*t, 0.0, 1.0);

    // Isophote rendering below - great for debugging.
    //    float x = cos(20.0*dot(n, normalize(eye_pos.rgb)));
    //    return vec3(x*x);
}

void main()
{
    vec3 r = normalize(ray_dir); // Ray direction
    vec3 p0; // ray starting point
    vec3 p1 = texture(gtex, gl_FragCoord.xy).xyz; // ray terminus
    float d; // pseudo-distance aka value of implicit
    
    // We test whether the normal in the pixel points towards the eye
    // or away from the eye.
    if (dot(norm,r) <-0.01) {
        // If the normal points toward the eye, the pixel contains the
        // front facing normal, and the ray origin is simply the position
        // of the pixel passed from the vertex shader.
        p0 = ray_origin;
        d = dist(p0);
        // If we are inside the implicit, we shade using the normal
        // stored in the pixel.
        if (d<0)  {
            fragColor.rgb = shade(p0, norm, 0);
            // Uncomment line below to clip bounding box as near plane.
            // fragColor.rgb = vec3(.2,0,0);
            return;
        }
    }
    else {
        // Otherwise, the near clipping plane intersects the triangle
        // geometry, and set the ray starting point to be on the near
        // clipping plane.
        p0 = eye_pos.rgb + r/dot(r, -normalize(eye_pos.rgb));   
        d = dist(p0);
        if (d<0)  {
            // Now, if we are inside the solid, we have clipped it and set
            // the color to a very very dark red.
            fragColor.rgb = vec3(.2,0,0);
            return;
        }
    }
    
    vec3 ray_span = p1-p0;
    r= normalize(ray_span);
    float t_max = length(ray_span);
    
    float t = 0.0; // distance along ray
    for (int i=0;i<10000;++i) {
        // Update ray parameter such that we take uniform small steps.
        // This is SLOW. Change it such that you use the distance value
        // and Lipschitz constant to choose a good step length
        t += 0.0002;
        
        // Compute position along ray.
        vec3 p = p0 + t * r;
        
        // Next, we compute the value of the implicit
        d = dist(p);

        // If the point p is now outside the domain and we did not hit the
        // surface, break out of the loop
        if(t>t_max && d>-thresh)
            break;
        
        // If we are sufficiently close to the surface, we set the
        // fragment color and return
        if(abs(d) < thresh) {
            fragColor.rgb = shade(p, normalize(dist_grad(p)), t);
            return;
        }
    }
    
    // We missed ... and set the fragment color to background color!
    fragColor.rgb = vec3(0.7);// * dot(vec2(0.5,0.5),smoothstep(0.45,0.55, fract(gl_FragCoord.xy / 30.0)));
}

