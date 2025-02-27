//
//  GLHeader.h
//  02564base
//
//  Created by Jakob Andreas Bærentzen on 11/02/2019.
//  Copyright © 2019 Jakob Andreas Bærentzen. All rights reserved.
//

#ifndef GLHeader_h
#define GLHeader_h

#include <iostream>

#define GLEW_STATIC
#include "GL/glew.h"

const GLubyte* opengl_error_string();

#define CHECK_GL_ERROR if(glGetError() != GL_NO_ERROR) std::cout << __FILE__ << __LINE__ << " error code: " << opengl_error_string() << std::endl;



#endif /* GLHeader_h */
