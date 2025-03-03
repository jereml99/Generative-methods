#include <iostream>

#include <CGLA/Mat4x4f.h>
#include <CGLA/Vec2f.h>
#include <CGLA/Vec3f.h>
#include <CGLA/Vec4f.h>
#include <GLGraphics/ThreeDObject.h>
#include <GLGraphics/User.h>

#include <GLGraphics/ShaderProgram.h>
#include <GLGraphics/ThreeDObject.h>
#include <GLGraphics/User.h>
#include <Mesh/Texmap.h>
#include <Mesh/TriangleMesh.h>
#include "GLFW/glfw3.h"

using namespace std;
using namespace CGLA;
using namespace GLGraphics;
using namespace Mesh;
using namespace std;
using namespace CGLA;
using namespace GLGraphics;

const string shader_path = "../shaders/PoolScene/";
const string objects_path = "../data/";


const float WATER_LEVEL = 0.5;
const int number_of_fish = 64;

User user(1.0);
GLuint background_tex = 0;
GLuint height_map_old_tex = 0;      // Height map used for water rendering
bool reload_shaders = true;         // This is set to true if the user presses 'r' then shaders are reloaded.
GLuint particle_data_tex = 0;       // Texture used for particles representing fish
float ax=0, ay=0, dist=0;
float ang_x=0;
float ang_y=0;
int mouse_x0=0,mouse_y0=0;
int WIN_WIDTH = 1024;
int WIN_HEIGHT = 768;
int FB_WIDTH, FB_HEIGHT;

GLGraphics::ShaderProgramDraw shader;
GLGraphics::ThreeDObject box;
GLuint m_vertexBuffer;


// ----------------------------------------
// OpenGL utility functions
// ----------------------------------------

TriangleMesh* build_quad( const Vec3f& p0,
                          const Vec3f& p1,
                          const Vec3f& p2,
                          const Vec3f& p3,
                          const Vec3f& normal,
                          const Vec2f& textScale = Vec2f(1,1),
                          const Vec3f& dpdu = Vec3f(0),
                          const Vec3f& dpdv = Vec3f(0),
                          ShaderProgram *shader = NULL){
    TriangleMesh* quad = new TriangleMesh();
    vector<Vec3f> vertices;
    vertices.push_back(p0);
    vertices.push_back(p1);
    vertices.push_back(p2);
    vertices.push_back(p3);
    quad->add("vertex", vertices);
    vector<Vec2f> texcoord;
    texcoord.push_back(Vec2f(0,0)*textScale);
    texcoord.push_back(Vec2f(0,1)*textScale);
    texcoord.push_back(Vec2f(1,0)*textScale);
    texcoord.push_back(Vec2f(1,1)*textScale);
    quad->add("texcoord", texcoord);
    vector<Vec3f> normals;
    for (int i=0;i<4;i++){
        normals.push_back(normal);
    }
    quad->add("normal", normals);
    vector<GLuint> indices;
    indices.push_back(0);
    indices.push_back(1);
    indices.push_back(2);
    indices.push_back(3);
    if (length(dpdu) > 0.0){
        vector<Vec3f> tangent;
        for (int i=0;i<4;i++){
            tangent.push_back(dpdu);
        }
        quad->add("dpdu", tangent);
    }
    if (length(dpdv) > 0.0){
        vector<Vec3f> bitangent;
        for (int i=0;i<4;i++){
            bitangent.push_back(dpdv);
        }
        quad->add("dpdv", bitangent);
    }
    Material mat; // unused material
    quad->add_draw_call(indices, mat, GL_TRIANGLE_STRIP);
    quad->build_vertex_array_object(shader);
    return quad;
}

void draw_screen_aligned_quad(ShaderProgram& shader_prog)
{
    static TriangleMesh* quad = 0;
    if (quad == 0){
        Vec3f normal(0,0,1);
        quad = build_quad(Vec3f(-1,-1,0), Vec3f(-1,1,0), Vec3f(1,-1,0), Vec3f(1,1,0), normal);
    }
    quad->renderDirect(shader_prog);
}


void set_light_and_camera(ShaderProgramDraw& shader_prog, bool is_reflected)
{
    static const Vec4f light_position(.3f,.3f,10.f,1.f);
    static const Vec4f light_specular(0.6f,0.6f,0.3f,0.6f);
    static const Vec4f light_diffuse(0.6f,0.6f,0.3f,0.6f);
    static const Vec4f light_ambient(0.3f,0.4f,0.6f,0.4f);

    Mat4x4f R = identity_Mat4x4f();

    if(is_reflected)
    {
        R[2][2] = -1;
        R = translation_Mat4x4f(Vec3f(0,0,WATER_LEVEL)) * R * translation_Mat4x4f(Vec3f(0,0,-WATER_LEVEL));
    }

    shader_prog.set_projection_matrix(perspective_Mat4x4f(55, float(WIN_WIDTH)/WIN_HEIGHT, 0.01f, 100.0f));
    shader_prog.set_view_matrix(user.get_view_matrix()*R);
    shader_prog.set_light_position(light_position);


    shader_prog.set_light_intensities(light_diffuse, light_specular, light_ambient);
}


GLuint create_texture(GLenum target, GLfloat min_filter, GLfloat mag_filter, GLfloat wrap)
{
    GLuint t;
    glGenTextures(1, &t);
    glBindTexture(target, t);
    glTexParameterf(target, GL_TEXTURE_MIN_FILTER, min_filter);
    glTexParameterf(target, GL_TEXTURE_MAG_FILTER, mag_filter);
    glTexParameterf(target, GL_TEXTURE_WRAP_S, wrap);
    glTexParameterf(target, GL_TEXTURE_WRAP_T, wrap);
    return t;
}

// ----------------------------------------
// Update fish and fluid solver
// ----------------------------------------

void update_flocking()
{
    static GLuint TEX_SIZE = 256;
    static int N = number_of_fish;
    static GLuint noise_tex, particle_tex[2];
    static GLuint fbo1=0;
    static GLuint particle_tex_no=1;
    if (!fbo1)
    {
        vector<float> noise_data(TEX_SIZE*TEX_SIZE*4);
        for (size_t i = 0; i < TEX_SIZE; ++i)
            for (size_t j = 0; j < TEX_SIZE; ++j)
            {
                Vec2f noise(gel_rand(), gel_rand());
                noise /= static_cast<float>(GEL_RAND_MAX);
                noise -= Vec2f(0.5, 0.5);
                noise *= 2.0;
                noise.normalize();
                noise_data[4*(TEX_SIZE*i+j)  ] = noise[0];
                noise_data[4*(TEX_SIZE*i+j)+1] = noise[1];
                noise_data[4*(TEX_SIZE*i+j)+2] = 0;
                noise_data[4*(TEX_SIZE*i+j)+3] = 0;
            }

        noise_tex = create_texture(GL_TEXTURE_2D, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, TEX_SIZE, TEX_SIZE, 0, GL_RGBA, GL_FLOAT, &(noise_data[0]));

        vector<float> data1(N*4);
        for (int i = 0; i < N; ++i)
        {
            data1[4*i  ] = 0.1 + 0.8*(float)(gel_rand())/GEL_RAND_MAX;
            data1[4*i+1] = 0.1 + 0.8*(float)(gel_rand())/GEL_RAND_MAX;
            data1[4*i+2] = 2.0*(0.5-(float)(gel_rand())/GEL_RAND_MAX);
            data1[4*i+3] = 2.0*(0.5-(float)(gel_rand())/GEL_RAND_MAX);
        }
        particle_tex[0] = create_texture(GL_TEXTURE_1D, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE);
        glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA32F, N, 0, GL_RGBA, GL_FLOAT, &(data1[0]));

        particle_tex[1] = create_texture(GL_TEXTURE_1D, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE);
        glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA32F, N, 0, GL_RGBA, GL_FLOAT, 0);

        glGenFramebuffers(1,&fbo1);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo1);
        glFramebufferTexture1D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_1D, particle_tex[0], 0);
        glFramebufferTexture1D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_1D, particle_tex[1], 0);
        if(glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            cout << "Something wrong with FBO" << endl;

    }
    static ShaderProgramDraw update_particles(shader_path,
                                   "$#version 150\nin vec3 vertex;in vec2 texcoord; out float _texcoord; void main() { _texcoord = texcoord.x; gl_Position = vec4(vertex,1); }",
                                   "", "update_fish.frag");


    if(reload_shaders)
        update_particles.reload();

    Vec2f u_pos = Vec2f(user.get_pos()[0], user.get_pos()[1]);
    u_pos += Vec2f(4.5, 4.5);
    u_pos /= 9.0;

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo1);

    glDrawBuffer(GL_COLOR_ATTACHMENT0 + particle_tex_no);
    glViewport(0,0,N,1);

    update_particles.use();
    update_particles.set_uniform("n_particles", N);
    update_particles.set_uniform("user_pos",u_pos);
    update_particles.use_texture(GL_TEXTURE_1D, "particles_tex", particle_tex[1-particle_tex_no]);
    update_particles.use_texture(GL_TEXTURE_2D, "noise_tex", noise_tex, 2);
    draw_screen_aligned_quad(update_particles);

    particle_data_tex = particle_tex[particle_tex_no];

    particle_tex_no = 1-particle_tex_no;
}

void fluid_solve(bool add_splash, const Vec2f& splash_center)
{
    static GLuint height_map_new_tex = 0;

    static int TEX_SIZE = 1024;
    static GLuint fbo;

    if (!height_map_old_tex)
    {
        vector<float> data(TEX_SIZE*TEX_SIZE*4);
        for (int i = 0; i < TEX_SIZE; ++i)
            for (int j = 0; j < TEX_SIZE; ++j)
            {
                data[4*(TEX_SIZE*i+j)  ] = 1.0;
                data[4*(TEX_SIZE*i+j)+1] = float(i)/TEX_SIZE;
                data[4*(TEX_SIZE*i+j)+2] = 0;
                data[4*(TEX_SIZE*i+j)+3] = 1.0;
            }
        height_map_old_tex = create_texture(GL_TEXTURE_2D, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, TEX_SIZE, TEX_SIZE, 0, GL_RGBA, GL_FLOAT, &(data[0]));

        height_map_new_tex = create_texture(GL_TEXTURE_2D, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, TEX_SIZE, TEX_SIZE, 0, GL_RGBA, GL_FLOAT, 0);

        glGenFramebuffers(1,&fbo);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, height_map_new_tex, 0);
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, height_map_old_tex, 0);
        if(glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            cout << "Something wrong with FBO" << endl;
    }

    static ShaderProgramDraw fluid_solver(shader_path,
                                      "$#version 150\nin vec3 vertex; in vec2 texcoord; out vec2 _texcoord; void main() { _texcoord = texcoord; gl_Position = vec4(vertex,1); }",
                                      "", "fluid_solver.frag");

    static ShaderProgramDraw pass_through(shader_path,
                                          "swap_and_splash.vert",
                                      "", "swap_and_splash.frag");

    if(reload_shaders)
    {
        fluid_solver.reload();
        pass_through.reload();
    }

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    glViewport(0,0,TEX_SIZE,TEX_SIZE);
    glClearColor(1,1,1,1);
    glClear(GL_COLOR_BUFFER_BIT);

    fluid_solver.use();
    fluid_solver.use_texture(GL_TEXTURE_2D, "h_sigma_tex", height_map_old_tex);

    draw_screen_aligned_quad(fluid_solver);

    glDrawBuffer(GL_COLOR_ATTACHMENT1);
    glViewport(0,0,TEX_SIZE,TEX_SIZE);
    glClearColor(0,0,1,1);
    glClear(GL_COLOR_BUFFER_BIT);


    pass_through.use();
    pass_through.use_texture(GL_TEXTURE_2D, "tex", height_map_new_tex);

    if (add_splash)
        glUniform1i(pass_through.get_uniform_location("add_splash"), 1);
    else
        glUniform1i(pass_through.get_uniform_location("add_splash"), 0);

    glUniform2fv(pass_through.get_uniform_location("splash_center"), 1, splash_center.get());

    draw_screen_aligned_quad(pass_through);

}


// ----------------------------------------
// The following functions draw the elements of the scene.
// ----------------------------------------

void draw_objects()
{
    static float t = 0.0;
    t += 0.1f;

    static ShaderProgramDraw fish(shader_path, "fish.vert", "", "fish.frag");
    if(reload_shaders)
        fish.reload();

    static vector<ThreeDObject> objects;
    if(objects.empty()) {
        ThreeDObject tdo;
        tdo.init(objects_path,"NeonTetra0.obj");
        for (int i = 0; i < number_of_fish; ++i)
        {
            objects.push_back(tdo);
            objects.back().translate(Vec3f(0.f,0.f,0.3f));
        }
    }

    fish.use();
    set_light_and_camera(fish, false);
    glUniform1f(fish.get_uniform_location("time"), t);
    fish.use_texture(GL_TEXTURE_1D, "particle_tex", particle_data_tex, 1);

    for(int i=0;i<objects.size();++i)
    {
        glUniform1f(fish.get_uniform_location("particle_no"), (i+0.5)/objects.size());
        objects[i].display(fish);
    }
}


void draw_room(bool is_reflected)
{
    static ShaderProgramDraw room(shader_path, "room.vert", "", "room.frag");
    static TriangleMesh *wall = 0;
    static TriangleMesh *floor = 0;
    if (wall == 0 || reload_shaders){
        room.reload();
        delete wall;
        delete floor;
        Vec3f normal(0, -1, 0);
        Vec2f texScale(3,1);
        Vec3f ddu = Vec3f(1,0,0);
        Vec3f ddv = Vec3f(0,0,1);
        wall = build_quad(Vec3f(-4.5,4.5,0), Vec3f(-4.5,4.5,3), Vec3f(4.5,4.5,0), Vec3f(4.5,4.5,3),
                          normal,
                          texScale,
                          ddu,
                          ddv, &room);
        normal = Vec3f(0, 0, 1);
        texScale = Vec2f(10,10);
        ddu = Vec3f(1,0,0);
        ddv = Vec3f(0,1,0);
        floor = build_quad(Vec3f(-4.5,-4.5,0), Vec3f(-4.5,4.5,0), Vec3f(4.5,-4.5,0), Vec3f(4.5,4.5,0),
                           normal,
                           texScale,
                           ddu,
                           ddv, &room);
    }

    static Texmap lion_tex(objects_path+"/Textures/lion.png");
    static Texmap lion_bump_tex(objects_path+"/Textures/lion-bump.png");
    static Texmap tiles_tex(objects_path+"/Textures/pale-tiles.jpg");
    static Texmap tiles_bump_tex(objects_path+"/Textures/pale-tiles-bump.jpg");


    room.use();
    set_light_and_camera(room, is_reflected);
    room.use_texture(GL_TEXTURE_2D, "tex", lion_tex.get_id());
    room.use_texture(GL_TEXTURE_2D, "hf_tex", lion_bump_tex.get_id(), 1);
    for(int i=0;i<4;++i)
    {
        room.set_model_matrix(rotation_Mat4x4f(ZAXIS, i*0.5*M_PI));
        wall->renderDirect(room);
    }

    if(!is_reflected)
    {
        //room.set_model_matrix(rotation_Mat4x4f(XAXIS, -0.5*M_PI));
        room.set_model_matrix(identity_Mat4x4f());
        room.use_texture(GL_TEXTURE_2D, "tex", tiles_tex.get_id());
        room.use_texture(GL_TEXTURE_2D, "tex_bump", tiles_bump_tex.get_id(), 1);
        floor->renderDirect(room);
    }

}


void draw_reflection()
{
    static GLuint tex=0, fbo=0, rb=0;
    static int _FB_WIDTH=-1;
    if(FB_WIDTH != _FB_WIDTH)
    {
        _FB_WIDTH=FB_WIDTH;
        glDeleteTextures(1, &tex);
        glDeleteFramebuffers(1, &fbo);
        glDeleteRenderbuffers(1, &rb);
        tex = create_texture(GL_TEXTURE_2D, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, FB_WIDTH, FB_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
        background_tex = create_texture(GL_TEXTURE_2D, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE);

        glGenFramebuffers(1,&fbo);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);

        glGenRenderbuffers(1,&rb);
        glBindRenderbuffer(GL_RENDERBUFFER, rb);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, FB_WIDTH, FB_HEIGHT);

        glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rb);

        if(glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            cout << "Something wrong with FBO" << endl;
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    }
    glBindTexture(GL_TEXTURE_2D, background_tex);
    glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, FB_WIDTH, FB_HEIGHT, 0);

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
    glViewport(0,0,FB_WIDTH, FB_HEIGHT);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    draw_room(true);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glDrawBuffer(GL_BACK);

    static ShaderProgramDraw render_water(shader_path, "render_water.vert", "", "render_water.frag");
    static TriangleMesh* waterQuad = 0;
    if(reload_shaders || waterQuad == 0) {
        render_water.reload();
        render_water.use();
        delete waterQuad;
        Vec3f normal(0,0,1);
        Vec3f ddu = Vec3f(1,0,0);
        Vec3f ddv = Vec3f(0,1,0);
        waterQuad = build_quad(Vec3f(-4.5,-4.5,WATER_LEVEL), Vec3f(4.5,-4.5,WATER_LEVEL), Vec3f(-4.5,4.5,WATER_LEVEL),
                               Vec3f(4.5,4.5,WATER_LEVEL), normal, Vec2f(1,1),ddu,ddv,&render_water);
        glUniform2f(render_water.get_uniform_location("win_dim"), FB_WIDTH, FB_HEIGHT);
    }
    render_water.use();
    render_water.use_texture(GL_TEXTURE_2D, "ref_tex", tex);
    render_water.use_texture(GL_TEXTURE_2D, "hf_tex", height_map_old_tex, 1);
    render_water.use_texture(GL_TEXTURE_2D, "bg_tex", background_tex, 2);
    set_light_and_camera(render_water, false);
    waterQuad->renderDirect(render_water);

}


struct ParticleData {Vec3f p; Vec4f v;};

void draw_particles()
{
    const int N = 3000;

    static ShaderProgramDraw particles(shader_path, "particles.vert", "particles.geom", "particles.frag");
    if(reload_shaders)
        particles.reload();
    static GLuint VAO=0;
    static GLuint VBO;
    if(VAO == 0)
    {
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);
        glGenBuffers(1, &VBO);

        vector<ParticleData> particle_vec;
        for(int i=0;i<N;++i)
        {
            ParticleData p;
            p.p = Vec3f(0.05f, 4.5f, 1.2f) + 0.02f*Vec3f(gel_rand(),gel_rand(),gel_rand())/float(GEL_RAND_MAX);
            p.v = Vec4f(gel_rand(),gel_rand(),0,gel_rand());
            p.v /= float(GEL_RAND_MAX);
            p.v *= Vec4f(0.2f,-0.4f, 0.0f, 0.5f);
            p.v -= Vec4f(0.1f,1.6f,0.f,0.f);
            particle_vec.push_back(p);
        }
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, particle_vec.size()*sizeof(ParticleData), &particle_vec[0], GL_STATIC_DRAW);
        GLuint vertex_attrib = particles.add_attrib("vertex");
        glEnableVertexAttribArray(vertex_attrib);
        glVertexAttribPointer(vertex_attrib, 3, GL_FLOAT, GL_FALSE, sizeof(ParticleData), 0);
        GLuint velocity_attrib = particles.add_attrib("velocity");
        glEnableVertexAttribArray(velocity_attrib);
        glVertexAttribPointer(velocity_attrib, 4, GL_FLOAT, GL_FALSE, sizeof(ParticleData), (const GLvoid*) sizeof(Vec3f));
        particles.link(); // attributes between VAO and shader.
    }
    // else

    set_light_and_camera(particles, false);
    const float delta_time = 0.01f;
    static float t = 0.0f;
    t+= delta_time;
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDepthMask(GL_FALSE);
    particles.set_uniform("time", t);
    particles.set_uniform("delta_time", delta_time);
    glBindVertexArray(VAO);
    particles.use();
    glDrawArrays(GL_POINTS, 0, N);
    glUseProgram(0);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);

}



void paintGL()
{
    update_flocking();
    Vec2f splash_center(0.925+(0.015*gel_rand()/float(GEL_RAND_MAX)-0.0025),4.55/9.0+(0.015*gel_rand()/float(GEL_RAND_MAX)-0.0025));
    fluid_solve(true, splash_center);
    for (int i = 0; i < 4; ++i)
        fluid_solve(false, splash_center);

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glDrawBuffer(GL_BACK);
    glViewport(0,0,FB_WIDTH, FB_HEIGHT);
    glClearColor(101.0f/255,69.0f/255,1.f,1.f);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    draw_room(false);
    draw_objects();
    draw_reflection();
    draw_particles();


    reload_shaders = false;
}

void initializeGL()
{
    glClearColor( 0.7f, 0.7f, 0.7f, 0.0f );
    glEnable(GL_DEPTH_TEST);
}

// Event handler called when the window is resized.
void window_size_callback(GLFWwindow* window, int _width, int _height)
{
    WIN_WIDTH = _width;
    WIN_HEIGHT = _height;
    reload_shaders = true;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    FB_WIDTH=width;
    FB_HEIGHT=height;
    glViewport(0, 0, FB_WIDTH, FB_HEIGHT);
}


// Event handler called when the cursor moves.
bool dragged_left_button = false;
static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
    float delta_ang_x = 4*M_PI*(mouse_x0-xpos)/WIN_WIDTH;
    float delta_ang_y = M_PI*(mouse_y0-ypos)/WIN_HEIGHT;
    float ax = ang_x + delta_ang_x;
    float ay = ang_y + delta_ang_y;
    Vec3f dir(cos(ax)*cos(ay),sin(ax)*cos(ay),sin(ay));
    user.set_dir(dir);
}

// Event handler called when a mouse button is pressed.
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    double x,y;
    glfwGetCursorPos(window, &x, &y);
    if (action == GLFW_PRESS) {
        mouse_x0 = x;
        mouse_y0 = y;
    }
    else if (action == GLFW_RELEASE) {
        float delta_ang_x = 4*M_PI*(mouse_x0-x)/WIN_WIDTH;
        float delta_ang_y = M_PI*(mouse_y0-y)/WIN_HEIGHT;
        ang_x += delta_ang_x;
        ang_y += delta_ang_y;
    }
}

// Event handler called when a keyboard key is pressed.
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if(action == GLFW_PRESS) {
        switch(key)
        {
            case 'W': user.forward();  break;
            case 'S': user.back();break;
            case 'A': user.strafe_left();break;
            case 'D': user.strafe_right();break;
        }
    }
    if(action == GLFW_RELEASE) {
        user.stop();
        switch(key)
        {
            case GLFW_KEY_ESCAPE: exit(0);
            case 'F':
                if( glfwGetWindowAttrib(window, GLFW_MAXIMIZED))
                    glfwRestoreWindow(window);
                else
                    glfwMaximizeWindow(window);
                break;
            case 'R':
                reload_shaders = true;
                break;
            default:
                user.stop();
                break;
        }
        
    }
}

// The main function
int main() {
    if (!glfwInit())
        return -1;
    
    // We ask for an OpenGL context specifically for version 4.1
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    /* Create a windowed mode window and its OpenGL context */
    GLFWwindow* window = glfwCreateWindow(WIN_WIDTH, WIN_HEIGHT, "Pool Scene", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetWindowSizeCallback(window, window_size_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    
    glfwMakeContextCurrent(window);
    GLenum glewinit = glewInit();
    if (glewinit != GLEW_OK) {
        std::cout << "Glew did not initialize: " << glewGetErrorString(glewinit) << endl;
        return -1;
    }
    else
        cout << "GLEW OK" << endl;
    initializeGL();
    glfwGetFramebufferSize(glfwGetCurrentContext(), &FB_WIDTH, &FB_HEIGHT);

    // Loop until the user closes the window
    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        user.update_position();
        paintGL();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    glfwTerminate();
    return 0;
}
