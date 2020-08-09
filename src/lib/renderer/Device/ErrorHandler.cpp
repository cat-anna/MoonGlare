#include "ErrorHandler.h"

namespace MoonGlare::Renderer::Device {

void DebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message,
                   const void *userParam) {
    const char *source_str = "?";
    const char *type_str = "?";
    const char *severity_str = "?";

    switch (source) {
    case GL_DEBUG_SOURCE_API_ARB:
        source_str = "OpenGL";
        break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB:
        source_str = "Windows";
        break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER_ARB:
        source_str = "Shader Compiler";
        break;
    case GL_DEBUG_SOURCE_THIRD_PARTY_ARB:
        source_str = "Third Party";
        break;
    case GL_DEBUG_SOURCE_APPLICATION_ARB:
        source_str = "Application";
        break;
    case GL_DEBUG_SOURCE_OTHER_ARB:
        source_str = "Other";
        break;
    default:
        source_str = "Unknown";
        break;
    }

    switch (type) {
    case GL_DEBUG_TYPE_ERROR_ARB:
        type_str = "Error";
        break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB:
        type_str = "Deprecated behavior";
        break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB:
        type_str = "Undefined behavior";
        break;
    case GL_DEBUG_TYPE_PORTABILITY_ARB:
        type_str = "Portability";
        break;
    case GL_DEBUG_TYPE_PERFORMANCE_ARB:
        type_str = "Performance";
        break;
    case GL_DEBUG_TYPE_OTHER_ARB:
        type_str = "Other";
        break;
    default:
        type_str = "Unknown";
        break;
    }

    switch (severity) {
    case GL_DEBUG_SEVERITY_HIGH_ARB:
        severity_str = "High";
        break;
    case GL_DEBUG_SEVERITY_MEDIUM_ARB:
        severity_str = "Medium";
        break;
    case GL_DEBUG_SEVERITY_LOW_ARB:
        severity_str = "Low";
        break;
    default:
        severity_str = "Unknown";
        break;
    }

    switch (severity) {
    case GL_DEBUG_SEVERITY_HIGH_ARB:
    case GL_DEBUG_SEVERITY_MEDIUM_ARB:
        AddLogf(Error, "OpenGL Error: Source:%s Type:%s ID:%d Severity:%s Message:%s", source_str, type_str, id,
                severity_str, message);
        return;
    case GL_DEBUG_SEVERITY_LOW_ARB:
    default:
        AddLogf(Warning, "OpenGL Warning: Source:%s Type:%s ID:%d Severity:%s Message:%s", source_str, type_str, id,
                severity_str, message);
        return;
    }
}

void ErrorHandler::RegisterErrorCallback() {
    if (GLEW_ARB_debug_output) {
        glDebugMessageCallbackARB(&DebugCallback, nullptr);

        // disable:
        // OpenGL Warning: Source:OpenGL Type:Other ID:131185 Severity:Unknown Message:Buffer detailed info: Buffer
        // object X (bound to GL_ARRAY_BUFFER_ARB, usage hint is GL_STATIC_DRAW) will use VIDEO memory as the source for
        // buffer object operations.
        GLuint skip1[] = {131185};
        glDebugMessageControlARB(GL_DEBUG_SOURCE_API_ARB, GL_DEBUG_TYPE_OTHER_ARB, GL_DONT_CARE, 1, skip1, GL_FALSE);

        AddLog(Debug, "Debug callback registerd");
    } else
        AddLog(Error, "Debug callback is not supported!");
}

} // namespace MoonGlare::Renderer::Device
