
#include "opengl_device_info.hpp"
#include "renderer/types.hpp"
#include <orbit_logger.h>
#include <thread>

namespace MoonGlare::Renderer {

void DeviceInfo::ReadInfo() {

    struct GLInfo_t {
        const char *Name;
        GLenum value;
        int type;
    };
#define _add(NAME, TYPE) {#NAME, NAME, TYPE},
    static const GLInfo_t GLInfo[] = {
        _add(GL_VERSION, 's')                  //
        _add(GL_SHADING_LANGUAGE_VERSION, 's') //
        _add(GL_RENDERER, 's')                 //

#ifdef GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT
        _add(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, 'f') //
#endif

        _add(GL_MAX_COLOR_ATTACHMENTS, 'i')            //
        _add(GL_MAX_TEXTURE_UNITS, 'i')                //
        _add(GL_MAX_TEXTURE_IMAGE_UNITS, 'i')          //
        _add(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, 'i')   //
        _add(GL_MAX_TEXTURE_SIZE, 'i')                 //
        _add(GL_MAX_DRAW_BUFFERS, 'i')                 //
        _add(GL_MAX_CUBE_MAP_TEXTURE_SIZE, 'i')        //
        _add(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, 'i') //
                                                       //
        _add(GL_MAX_VERTEX_UNIFORM_VECTORS, 'i')       //
                                                       //
        _add(GL_MINOR_VERSION, 'i')                    //
        _add(GL_MAJOR_VERSION, 'i')                    //
        {},
    };
#undef _add

    /*	GLenum params[] ={
    GL_MAX_FRAGMENT_UNIFORM_COMPONENTS,
    GL_MAX_VARYING_FLOATS,
    GL_MAX_VERTEX_ATTRIBS,
    GL_MAX_VERTEX_UNIFORM_COMPONENTS,
    GL_MAX_VIEWPORT_DIMS,//2d integer
    GL_STEREO,//boolean
    };*/

    for (auto *it = GLInfo; it->Name; ++it) {
        switch (it->type) {
        case 's': {
            const char *text = (const char *)glGetString(it->value);
            if (!text)
                AddLog(Warning, "Unable to get OpenGL string " << it->Name);
            else
                AddLogf(System, "%s = %s", it->Name, text);
            break;
        }
        case 'f': {
            float f;
            glGetFloatv(it->value, &f);
            AddLogf(System, "%s = %f", it->Name, f);
            break;
        }
        case 'i': {
            int i;
            glGetIntegerv(it->value, &i);
            AddLogf(System, "%s = %d", it->Name, i);
            break;
        }
        }
    }

    AddLogf(System, "CPU threads = %u", std::thread::hardware_concurrency());
}

} // namespace MoonGlare::Renderer
