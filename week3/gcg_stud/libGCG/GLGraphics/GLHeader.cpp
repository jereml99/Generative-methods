//
//  GLHeader.cpp
//  02564base
//
//  Created by Andreas Bærentzen on 11/02/2019.
//  Copyright © 2019 Jakob Andreas Bærentzen. All rights reserved.
//

#include "GLHeader.h"

#ifdef __APPLE__
#include <OpenGL/glu.h>
#endif

const GLubyte* opengl_error_string() {
    return gluErrorString(glGetError());
}
