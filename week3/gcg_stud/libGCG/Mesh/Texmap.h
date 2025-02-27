#ifndef __TEXMAP_H__
#define __TEXMAP_H__

#include <iostream>
#include <string>
#include "CGLA/Vec3f.h"
#include <GLGraphics/GLHeader.h>
#include "../SOIL/SOIL.h"

namespace Mesh
{
    /// A simple texture map class.
	class Texmap 
	{
        
        GLuint id = 0;
        std::string name = "";
        
	public:
        
		// Constuctor/destructor
        Texmap() {}
        Texmap(const std::string& _name) {
            bool loadSucces = load(_name);
            if (!loadSucces){
                std::cerr << "Unable to load "<<_name.c_str();
            }
        }
        ~Texmap() {}
        
		/// get the texture name.
		const std::string& get_name() const {return name;}
        
		/// Load with a given file name
		bool load(const std::string& _name);
                
		/// Initializes the texture wrt OpenGL.
		void gl_init();

        GLuint get_id() {if(!id) gl_init(); return id;}
	};
    
}
#endif



