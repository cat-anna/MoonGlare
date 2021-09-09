#include "opengl_error_handler.hpp"
#include "renderer/types.hpp"
#include <glad/glad.h>
// #include <gl/glew.h>

namespace MoonGlare::Renderer {

void DebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
                   const GLchar *message, const void *userParam) {
    const char *source_str = "?";
    const char *type_str = "?";
    const char *severity_str = "?";

    switch (source) {
    case GL_DEBUG_SOURCE_API:
        source_str = "OpenGL";
        break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
        source_str = "Windows";
        break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER:
        source_str = "Shader Compiler";
        break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:
        source_str = "Third Party";
        break;
    case GL_DEBUG_SOURCE_APPLICATION:
        source_str = "Application";
        break;
    case GL_DEBUG_SOURCE_OTHER:
        source_str = "Other";
        break;
    default:
        source_str = "Unknown";
        break;
    }

    switch (type) {
    case GL_DEBUG_TYPE_ERROR:
        type_str = "Error";
        break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        type_str = "Deprecated behavior";
        break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        type_str = "Undefined behavior";
        break;
    case GL_DEBUG_TYPE_PORTABILITY:
        type_str = "Portability";
        break;
    case GL_DEBUG_TYPE_PERFORMANCE:
        type_str = "Performance";
        break;
    case GL_DEBUG_TYPE_OTHER:
        type_str = "Other";
        break;
    default:
        type_str = "Unknown";
        break;
    }

    switch (severity) {
    case GL_DEBUG_SEVERITY_HIGH:
        severity_str = "High";
        break;
    case GL_DEBUG_SEVERITY_MEDIUM:
        severity_str = "Medium";
        break;
    case GL_DEBUG_SEVERITY_LOW:
        severity_str = "Low";
        break;
    default:
        severity_str = "Unknown";
        break;
    }

    switch (severity) {
    case GL_DEBUG_SEVERITY_HIGH:
    case GL_DEBUG_SEVERITY_MEDIUM:
        AddLogf(Error, "OpenGL Error: Source:%s Type:%s ID:%d Severity:%s Message:%s", source_str,
                type_str, id, severity_str, message);
        return;
    case GL_DEBUG_SEVERITY_LOW:
    default:
        AddLogf(Warning, "OpenGL Warning: Source:%s Type:%s ID:%d Severity:%s Message:%s",
                source_str, type_str, id, severity_str, message);
        return;
    }
}

void ErrorHandler::RegisterErrorCallback() {
    if (glDebugMessageCallback && glDebugMessageControl) {
        glDebugMessageCallback(&DebugCallback, nullptr);

        // disable:
        // OpenGL Warning: Source:OpenGL Type:Other ID:131185 Severity:Unknown Message:Buffer detailed info: Buffer
        // object X (bound to GL_ARRAY_BUFFER, usage hint is GL_STATIC_DRAW) will use VIDEO memory as the source for
        // buffer object operations.
        GLuint skip1[] = {131185};
        glDebugMessageControl(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_OTHER, GL_DONT_CARE, 1, skip1,
                              GL_FALSE);

        AddLog(Debug, "Debug callback registered");
    } else {
        AddLog(Error, "Debug callback is not supported!");
    }
}

void ErrorHandler::CheckError() {
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        // auto ptr = (char *)gluErrorString(err);
        // if (ptr) {
        //     AddLog(Error, "OpenGL error: " << ptr);
        // } else {
        AddLog(Error, "OpenGL error code " << err);
        // }
    }
}

} // namespace MoonGlare::Renderer
