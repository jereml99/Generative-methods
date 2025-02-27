/*
 *  User.h
 *  02564_Framework
 *
 *  Created by J. Andreas Bærentzen on 02/02/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef __USER_H__
#define __USER_H__

#include <CGLA/Vec4f.h>
#include <CGLA/Vec3f.h>
#include <CGLA/Vec2f.h>
#include <CGLA/Mat4x4f.h>
#include <cmath>
#include <GLGraphics/ShaderProgram.h>

#undef min
#undef max

namespace GLGraphics {
    
    
    class HeightMap 
    {
    public:
        virtual float height(const CGLA::Vec3f&) const {return 0.5;}
    };
    
    const HeightMap* default_height_map();
    
    /** A simple class encapsulating position and direction of user */
    class User
    {
        CGLA::Vec3f pos = CGLA::Vec3f(0.0);
        CGLA::Vec3f dir = CGLA::normalize(CGLA::Vec3f(1,1,0));
        CGLA::Vec2f speed = CGLA::Vec2f(0,0);
        const HeightMap* terra = nullptr;
        bool fly_mode = false;
        float elevation = 2.0f;
        
    public:
        /** Construct by passing a terrain. The user is always on the terrain, so he needs to 
         query the height function. */
        User(const HeightMap* _terra, float _elevation = 2.0): terra(_terra), elevation(_elevation)
        {
            pos = CGLA::Vec3f(256,256,terra->height(CGLA::Vec3f(256,256,64)));
        }
        
        User(float _elevation = 2.0): elevation(_elevation)
        {
            terra = default_height_map();
        }

        void toggle_fly_mode() {fly_mode = !fly_mode;}
        
        /// Move forward, project to terrain.
        void update_position() 
        {
            if (fly_mode)
                pos += speed[0] * dir;
            else {
                pos += speed[0] * normalize(CGLA::Vec3f(dir[0],dir[1],0));
                pos += speed[1] * normalize(CGLA::Vec3f(-dir[1],dir[0],0));
                float h = terra->height(pos);
                pos[2] = std::max(elevation,elevation + h);
            }
        }
        
        /// Get position
        const CGLA::Vec3f& get_pos() const {return pos;}
        
        /// Get direction
        const CGLA::Vec3f& get_dir() const {return dir;}
        
        /// Don't move!
        void stop() {speed = CGLA::Vec2f(0,0);}
        
        /// Go forward
        void forward() {speed = CGLA::Vec2f(0.25,0);}
        
        ///
        void run() {speed = CGLA::Vec2f(0.5,0);}

        /// Go back
        void back() {speed = CGLA::Vec2f(-0.1,0);}
        
        /// Move directly sideways left
        void strafe_left() {speed = CGLA::Vec2f(0,.15);}
        
        /// Move directly sideways right
        void strafe_right() {speed = CGLA::Vec2f(0,-.15);}
        
        /// Set orientation (in 2D).
        void set_dir(int mx, int my)
        {
            CGLA::Vec3f _dir;
            dir += -0.003*CGLA::Vec3f(-dir[1],dir[0],0)*mx+ 0.003*CGLA::Vec3f(0,0,1)*my;
            dir.normalize();	
        }
        
        /// Set position (in 2D).
        void set(const CGLA::Vec3f& _pos, const CGLA::Vec3f& _dir)
        {
            pos = _pos;
            dir = _dir;
        }
        
        /// Set orientation in 3D.
        void set_dir(const CGLA::Vec3f& _dir)
        {
            dir = _dir;
        }
        
        CGLA::Mat4x4f get_view_matrix() const
        {
            return CGLA::lookAt_Mat4x4f(pos, pos+dir, CGLA::Vec3f(0,0,1));
        }
        
    };
}

#endif 
