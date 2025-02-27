#include <iostream>
#include "../CGLA/Vec2i.h"
#include "../CGLA/Vec2f.h"
#include "../SOIL/SOIL.h"
#include "Texmap.h"

using namespace std;
using namespace CGLA;

namespace Mesh
{    
    
	bool Texmap::load(const std::string& _name)
	{
        name = _name;
        id = SOIL_load_OGL_texture(name.data(), 4, 0, SOIL_FLAG_TEXTURE_REPEATS | SOIL_FLAG_INVERT_Y);
        return id != 0;
    }
		
    
    void Texmap::gl_init()
    {
//        glGenTextures(1,&id);
//        glBindTexture(GL_TEXTURE_2D, id);
//        if(image.is_empty())
//        {
//            GLubyte dummy[] = {255,255,255,255};
//            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1,0,GL_RGBA,GL_UNSIGNED_BYTE,dummy);
//        }
//        else
//        {
//            size_t channels = image.spectrum() == 3 ? GL_RGB : GL_RGBA;
//            glTexImage2D(GL_TEXTURE_2D, 0, channels, image.width(), image.height(),0,channels,GL_UNSIGNED_BYTE,image.data());
//        }
//        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
//        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }
    
}	
