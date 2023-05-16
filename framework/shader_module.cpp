#include "shader_module.h"
#include <cassert>

namespace vk_engine
{
    void ShaderSource::load(const std::string &filepath)
    {
        auto len = filepath.length();
        assert(len > 4);
        if( 0 == filepath.compare(len-4, 4, ".spv"))
        {
            if(0 == filepath.compare(len-9, 9, ".vert.spv"))
            {

            } else if(0 == filepath.compare(len-9, 9, ".frag.spv"))
            {

            } else if(0 == filepath.compare(len-9, 9, ".comp.vert"))
            {

            }
        } else {
            
        }
    }
}