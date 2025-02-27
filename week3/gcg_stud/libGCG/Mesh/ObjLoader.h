#ifndef __OBJ_LOADER_H
#define __OBJ_LOADER_H

#include <string>
#include <vector>
#include <CGLA/Vec3f.h>
#include <CGLA/Vec2f.h>
#include <Mesh/Material.h>

namespace Mesh {
    
    class ObjLoader
    {
    public:
        /** Load an OBJ model into the out parameters.
         Note that an OBJ file may both consist of and link to other types of information than just
         geometry. The file itself may contain normals and texture coordinates in addition to vertex positions.
         Also, a material database may be associated with the OBJ file (.mtl) and materials can in turn
         reference texture images. The out_indices argument will contain a vector of indices upon return.
         Each index vector corresponds to a material because we need to render each material separately. */
        static bool load_object(const std::string& path,
                                const std::string& filename,
                                std::vector<CGLA::Vec3f> *out_positions,
                                std::vector<std::vector<GLuint> > *out_indices,
                                std::vector<Mesh::Material> *out_materials,
                                std::vector<CGLA::Vec3f> *out_normal = NULL,
                                std::vector<CGLA::Vec2f> *out_uv = NULL,
                                float scale = 1.0f
                                );
        
    };
}

#endif
