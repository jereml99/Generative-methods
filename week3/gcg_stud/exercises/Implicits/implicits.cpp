/*
 * IntroScene: This is a simple program which allows you to explore and understand the 02564 framework with less
 * pain than if you had a bigger code base.
 *
 *  User interaction:
 *     Mouse rotates,
 *     arrow up and down zooms in and out,
 *     [ESC] quits,
 *     'r' reloads shaders.
 *
 */

#include <iostream>
#include <string>

#include <CGLA/Mat4x4f.h>
#include <CGLA/Vec3f.h>
#include <CGLA/Vec4f.h>
#include <CGLA/Quatf.h>
#include "GLGraphics/ShaderProgram.h"
#include "GLGraphics/ThreeDObject.h"
#include "GBufferLight.h"
#include "GLFW/glfw3.h"

using namespace std;
using namespace CGLA;
using namespace GLGraphics;

const string shader_path = "../shaders/Implicits/";
const string objects_path = "../data/cube/";

int WIN_WIDTH = 1024;
int WIN_HEIGHT = 768;
int FB_WIDTH = -1;
int FB_HEIGHT = -1;
float FOV = 55;
double dist = 3.5, ang_x = 0, ang_y = 0;
double mouse_x0=0, mouse_y0=0, ax=0,ay=0;
double trans_x=0, trans_y=0;
ShaderProgramDraw shader, shader_back_face;
ThreeDObject box;
bool spin = false;

// Load the shaders and set material parameters.
void load_shaders()
{
    shader.init(shader_path, "object.vert", "", "object.frag");
    shader.use();
    shader.set_material(Vec4f(.1,.1,.1,1),Vec4f(.9,.9,.9,1),128);
    shader_back_face.init(shader_path,
                          "object_back_face.vert", "",
                          "object_back_face.frag");
}


// Initialize the app: load shaders, content, and initialize OpenGL
void initialize()
{
    load_shaders();
	cout << "Loaded shaders" << endl;
    box.init(objects_path, "cube.obj");
    // box.init(objects_path, "bunny.obj");
    // box.init(objects_path, "canti.obj");
    glClearColor( 0.7f, 0.7f, 0.7f, 0.0f );
    glEnable(GL_DEPTH_TEST);
}

// Setup lighting and cameras. Called every frame.
void set_light_and_camera()
{
    const float near = 1.0;
    const float far = 100.0;
    static const Vec4f light_specular(0.6,0.6,0.3,0.6);
    static const Vec4f light_diffuse(0.6,0.6,0.3,0.6);
    static const Vec4f light_ambient(0.3,0.4,0.6,0.4);

    
    
    if(spin) ang_x += 0.001;
    
    Mat4x4f V =
            translation_Mat4x4f(Vec3f(trans_x,-trans_y,- dist))
            *rotation_Mat4x4f(XAXIS, ang_y)
            *rotation_Mat4x4f(YAXIS, ang_x);
    
    Mat4x4f VI = invert(V);
    Vec4f eye_pos = VI * Vec4f(0,0,0,1);
    Vec3f y_dir = VI.mul_3D_vector(Vec3f(0,1,0));
    Mat4x4f P = perspective_Mat4x4f(FOV, double(WIN_WIDTH)/WIN_HEIGHT, near, far);
    
    shader_back_face.use();
    shader_back_face.set_projection_matrix(P);
    shader_back_face.set_view_matrix(V);

    shader.use();
    shader.set_projection_matrix(P);
    shader.set_light_position(Vec4f(0,0,1,0));
    shader.set_view_matrix(V);
    shader.set_uniform("WIN_SCALE", Vec2f(FB_WIDTH, FB_HEIGHT)/2.0);
    shader.set_uniform("eye_pos", eye_pos);
    shader.set_uniform("y_dir", y_dir);
    shader.set_uniform("gtex", 0);
}

// Draw the scene.
void paintGL()
{
    static GBuffer gbuff(FB_WIDTH, FB_HEIGHT);
    static GLuint timer_query = -1;
    static int frames=0;
    static vector<float> msecs_total(100);
    static float T = 0.0;
    gbuff.rebuild_on_resize(FB_WIDTH, FB_HEIGHT);
    if(timer_query == -1)
        glGenQueries(1,&timer_query);
    
    glFinish();
    glBeginQuery(GL_TIME_ELAPSED, timer_query);
    set_light_and_camera();

    gbuff.enable();
    glClearDepth(0);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    shader_back_face.use();
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glDepthFunc(GL_GREATER);
    box.display(shader_back_face);
    
    
    glDisable(GL_CULL_FACE);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glViewport(0, 0, FB_WIDTH, FB_HEIGHT);
    glDrawBuffer(GL_BACK);
    shader.use();
    glClearColor(.7,.7,.7,.7);
    glClearDepth(1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDepthFunc(GL_LESS);
    gbuff.bind_textures(0, -1, -1);
    shader.set_uniform("T", T);
    box.display(shader);
    glFinish();
    glEndQuery(GL_TIME_ELAPSED);
    int nano_secs;
    glGetQueryObjectiv(timer_query, GL_QUERY_RESULT, &nano_secs);
    float m_secs = nano_secs/(1.0e6);
    T += nano_secs * 1e-9;
    msecs_total[frames++] = m_secs;
    if (frames == 100)
    {
        nth_element(msecs_total.begin(), msecs_total.begin()+50, msecs_total.end());
            cout << "median frame time (ms): " << msecs_total[50] << endl;
        frames = 0;
    }
}

// Event handler called when the window is resized.
void window_size_callback(GLFWwindow* window, int width, int height)
{
    WIN_WIDTH=width;
    WIN_HEIGHT=height;
}

// Event handler called when the frame buffer is resized
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    FB_WIDTH=width;
    FB_HEIGHT=height;
    glViewport(0, 0, FB_WIDTH, FB_HEIGHT);
}


// Event handler called when the cursor moves.
bool dragged_left_button = false;
bool dragged_right_button = false;
static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
    double x,y;
    glfwGetCursorPos(window, &x, &y);
    if(dragged_left_button) {
        float delta_ang_x = -M_PI*(x-mouse_x0)/float(WIN_WIDTH);
        float delta_ang_y = -M_PI*(y-mouse_y0)/float(WIN_WIDTH);
        ang_x = ang_x + delta_ang_x;
        ang_y = ang_y + delta_ang_y;
        mouse_x0 = x;
        mouse_y0 = y;
    }
    else if (dragged_right_button) {
        trans_x += 2*(x-mouse_x0)/float(WIN_WIDTH);
        trans_y += 2*(y-mouse_y0)/float(WIN_WIDTH);
        mouse_x0 = x;
        mouse_y0 = y;
    }
}

// Event handler called when a mouse button is pressed.
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    double x,y;
    glfwGetCursorPos(window, &x, &y);
    if (action == GLFW_PRESS) {
        mouse_x0 = x;
        mouse_y0 = y;
        if (button == GLFW_MOUSE_BUTTON_LEFT)
            dragged_left_button = true;
        else
            dragged_right_button = true;
    }
    else {
        dragged_left_button = false;
        dragged_right_button = false;
    }
}

// Event handler called when a keyboard key is pressed.
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if(action == GLFW_RELEASE)
        switch(key) {
            case GLFW_KEY_ESCAPE: exit(0);
            case GLFW_KEY_UP: dist /= 1.1; break;
            case GLFW_KEY_DOWN: dist *= 1.1; break;
            case 'R': load_shaders(); break;
            case 'S': spin = !spin; break;
            case 'C': FOV /= 2.0; break;
            case 'F': FOV *= 2.0; break;
        }
}

// The main function
int main() {
    if (!glfwInit())
        return -1;
    
    // We ask for an OpenGL context specifically for version 4.1
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
//    glfwWindowHint(GLFW_SAMPLES, 32);
    
    /* Create a windowed mode window and its OpenGL context */
    GLFWwindow* window = glfwCreateWindow(WIN_WIDTH, WIN_HEIGHT, "Implicits", NULL, NULL);
    if (!window) {
		std::cout << "GLFW failed to create window " << endl;
		glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Below, we initialized extensions. This is required if our gl.h does not contain
    // Modern OpenGL functions.
	GLenum glewinit = glewInit();
	if (glewinit != GLEW_OK) {
		std::cout << "Glew did not initialize: " << glewGetErrorString(glewinit) << endl;
		return -1;
	}
	else
		cout << "GLEW OK" << endl;

    // Setup all of the event handlers.
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetWindowSizeCallback(window, window_size_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glfwMakeContextCurrent(window);
    initialize();
    glfwGetFramebufferSize(glfwGetCurrentContext(), &FB_WIDTH, &FB_HEIGHT);
    
//    glEnable(GL_MULTISAMPLE);

    // Loop until the user closes the window
    while (!glfwWindowShouldClose(window))
    {
        paintGL();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    glfwTerminate();
    return 0;
}
