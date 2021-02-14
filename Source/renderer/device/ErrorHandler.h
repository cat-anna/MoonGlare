#pragma once

#include <gl/glew.h>
#include <orbit_logger.h>

namespace MoonGlare::Renderer::Device {

struct ErrorHandler {
    static void RegisterErrorCallback();

    static void CheckError() {
        GLenum err = glGetError();
        if (err != GL_NO_ERROR) {
            auto ptr = (char *)gluErrorString(err);
            if (ptr)
                AddLog(Error, "OpenGL error: " << ptr);
            else
                AddLog(Error, "OpenGL error code " << err);
        }
    }
};

} // namespace MoonGlare::Renderer::Device
