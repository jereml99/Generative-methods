/*
 *  shaders.h
 *  02564_Framework
 *
 *  Created by J. Andreas Bærentzen on 31/01/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef __SHADERPROGRAM_H__
#define __SHADERPROGRAM_H__

#include <string>

#include <CGLA/Mat4x4f.h>
#include <CGLA/Mat3x3f.h>
#include <CGLA/Vec2f.h>
#include <CGLA/Vec3f.h>
#include <CGLA/Vec4f.h>

#include "GLHeader.h"

namespace GLGraphics {

    const std::vector<std::string>& get_generic_attrib_vec();


    /** A generic class for loading and enabling shader programs. Construct with the file names of
     shaders. */
    class ShaderProgram
    {
        std::string shader_path = "";  // as the name says
        std::string vs_file_name = ""; // Vertex shader file name
        std::string gs_file_name = ""; // geometry shader file name
        std::string fs_file_name = ""; // Fragment shader file name
        GLuint vs= 0, gs = 0, fs = 0; // The shaders

        static int currentShader;
        
    protected:
        std::vector<std::string> attrib_vec;

        GLuint prog; // The program

        void compile();


        /// Destroy shaders and program
        void relinquish();
        
    public:
        
        GLuint id() const {return prog;}

        ShaderProgram() {}

        /// Construct from file names. Note that geometry shader can be omitted by giving an empty string.
        ShaderProgram(const std::string& _shader_path,
                      const std::string& _vs_file_name,
                      const std::string& _gs_file_name,
                      const std::string& _fs_file_name,
                      const std::vector<std::string>& _attrib_vec = std::vector<std::string>()):
        shader_path(_shader_path),
        vs_file_name(_vs_file_name), 
        gs_file_name(_gs_file_name), 
        fs_file_name(_fs_file_name),
        attrib_vec(_attrib_vec)
        {
            reload();
        }
        
        void init(const std::string& _shader_path,
                  const std::string& _vs_file_name,
                  const std::string& _gs_file_name,
                  const std::string& _fs_file_name){
            shader_path = _shader_path;
            vs_file_name = _vs_file_name;
            gs_file_name = _gs_file_name;
            fs_file_name = _fs_file_name;
            reload();
        }

        /// Destroy and relinquish.
       virtual ~ShaderProgram()
        {
            relinquish();
        }

        /// Link or relink the shader. Useful if attributes have been bound.
        void link();

        
        /// Reload shaders that make up program. Nifty if changed on disk.
        void reload();
        
        /// Enable shader. No disable since we might not want to switch back to previous state.
        void use();
        
        /** Returns uniform location. Much nicer for me than maintaining many functions that help _set_ 
         uniforms. Minimal extra overhead for user. */
        GLint get_uniform_location(const std::string& name);

        /// Set uniforms in many different ways. Nicer for the user ostensibly.
        void set_uniform(const char* name, int value);
        void set_uniform(const char* name, float value);
        void set_uniform(const char* name, const CGLA::Vec2f &value);
        void set_uniform(const char* name, const CGLA::Vec3f &value);
        void set_uniform(const char* name, const CGLA::Vec4f &value);
        void set_uniform(const char* name, const CGLA::Mat4x4f &value);
        void set_uniform(const char* name, const CGLA::Mat3x3f &value);

        /// Get attribute location. Call only when you positively know it has been defined.
        GLint get_attrib_location(const std::string& name) const {
            for(int i=0;i<attrib_vec.size();++i)
                if (attrib_vec[i]==name) return i;
            return -1;
        }

        /** Add an attribute to the set of attributes. Need to call link after this and before rendering.
         Note that the shader can be linked multiple times. */
        GLint add_attrib(const std::string& name) {
            int N = get_attrib_location(name);
            if(N != -1)
                return N;
            N = attrib_vec.size();
            attrib_vec.push_back(name);
            return N;
        }
        
        /** Get the vector of vertex shader attributes. Useful if your attributes are to be transferred to
        another shader program */
        std::vector<std::string> get_attrib_vec() const {return attrib_vec;}
        
        /// Directly assign the attribute vector.
        void set_attrib_vec(const std::vector<std::string>& _attrib_vec) { attrib_vec = _attrib_vec;}
        
        /// Use a texture. The arguments correspond directly to OpenGL entities except for name.
        void use_texture(GLenum target, const std::string& name, GLuint tex, GLuint active_texture=0);
        
        /**
         * @brief load_a_shader load and compiles a GLSL shader
         * @param shader_type GL_VERTEX_SHADER, GL_GEOMETRY_SHADER or GL_FRAGMENT_SHADER
         * @param shader_path path of shaders (must end with '/')
         * @param str shader file name (or if string starts with '$' then str is used as shader source)
         * @return shader handle or 0 if not created
         */
        static GLuint load_a_shader(GLenum shader_type, const std::string& shader_path, const std::string& str);

        /** Create a shader of type specified by the first argument from a source string given
            as second argument.	Return shader handle. If there is a problem, the infolog is
            printed and 0 is returned. */
        static GLuint create_glsl_shader(GLuint stype, const std::string& src);
    };
    
    
    class ShaderProgramDraw: public ShaderProgram
    {        
        CGLA::Vec4f light_pos = CGLA::Vec4f(0.0f,0.0f,1.0f,0.0f);
        CGLA::Vec4f light_diff = CGLA::Vec4f(0.6f);
        CGLA::Vec4f light_spec = CGLA::Vec4f(0.2f);
        CGLA::Vec4f light_amb = CGLA::Vec4f(0.2f);
        
        CGLA::Vec4f mat_diff = CGLA::Vec4f(0.6f);
        CGLA::Vec4f mat_spec = CGLA::Vec4f(0.4f);
        float mat_spec_exp = 32;
        
        CGLA::Mat4x4f M = CGLA::identity_Mat4x4f();
        CGLA::Mat4x4f V = CGLA::identity_Mat4x4f();
        CGLA::Mat4x4f P = CGLA::identity_Mat4x4f();
        
        
        void set_derived_matrices();
    public:
        static GLint get_generic_attrib_location(const std::string& name)  {
            const auto& generic_attrib_vec = get_generic_attrib_vec();
            for(int i=0;i<generic_attrib_vec.size();++i)
                if (generic_attrib_vec[i]==name) return i;
            return -1;
        }

        
        
        ShaderProgramDraw(){
            ShaderProgram::attrib_vec = get_generic_attrib_vec();
        }
        
        /// Construct from file names. Note that geometry shader can be omitted by giving an empty string.
        ShaderProgramDraw(const std::string& _shader_path,
                          const std::string& _vs_file_name,
                          const std::string& _gs_file_name,
                          const std::string& _fs_file_name,
                          const std::vector<std::string>& _attrib_vec = std::vector<std::string>()):
        ShaderProgram(_shader_path, _vs_file_name, _gs_file_name, _fs_file_name, get_generic_attrib_vec()) {
            ShaderProgram::attrib_vec.insert(attrib_vec.end(), _attrib_vec.begin(), _attrib_vec.end());
        }

        /// Destroy and relinquish.
        ~ShaderProgramDraw() {}
        
        /*
         * The following functions set various state pertaining to normal light and material shaders.
         * This all goes into uniforms. Call this function only after the shader has been linked.
         */
        
        void set_light_position(const CGLA::Vec4f& light_pos);
        
        void set_light_intensities(const CGLA::Vec4f& diff, const CGLA::Vec4f& spec, const CGLA::Vec4f& amb);
        
        void set_material(const CGLA::Vec4f& diff, const CGLA::Vec4f& spec, float exp);
        
        void set_model_matrix(const CGLA::Mat4x4f& model_matrix);
        
        void set_view_matrix(const CGLA::Mat4x4f& view_matrix);
        
        void set_projection_matrix(const CGLA::Mat4x4f& projection_matrix);
        
        CGLA::Mat4x4f get_view_matrix() const {return V;}
        
        CGLA::Mat4x4f get_model_matrix() const {return M;}
        
        CGLA::Mat4x4f get_projection_matrix() const {return P;}
        
    };
}


#endif
