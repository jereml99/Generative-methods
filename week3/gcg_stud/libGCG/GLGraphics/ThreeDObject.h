#ifndef __THREE_D_OBJECT_H__
#define __THREE_D_OBJECT_H__

/*
 *  ThreeDObject.h
 *  02564_Framework
 *
 *  Created by J. Andreas Bærentzen on 02/02/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include <CGLA/Vec3f.h>
#include "ShaderProgram.h"
#include <Mesh/TriangleMesh.h>
#include <string>

namespace GLGraphics
{

class ThreeDObject
{
    Mesh::TriangleMesh mesh;

    CGLA::Vec3f translation_vector = CGLA::Vec3f(0);
    CGLA::Vec3f rotation_axis = CGLA::Vec3f(1,0,0);
    float rotation_angle = 0.0;
    CGLA::Vec3f scaling_factors = CGLA::Vec3f(1);

public:

    ThreeDObject() {}

    bool init(const std::string& path, const std::string& filename);

    void rotate(const CGLA::Vec3f& axis, float angle)
    {
        rotation_axis = axis;
        rotation_angle = angle;
    }

    void translate(const CGLA::Vec3f& pos)
    {
        translation_vector = pos;
    }

    void scale(const CGLA::Vec3f& scale)
    {
        scaling_factors  = scale;
    }

    void display(ShaderProgramDraw&);

};

}

#endif
