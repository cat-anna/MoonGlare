
#include "shader_init_commands.hpp"
#include "renderer/types.hpp"
#include <cassert>
#include <fmt/format.h>
#include <glad/glad.h>
#include <orbit_logger.h>
#include <stdexcept>

namespace MoonGlare::Renderer::Commands {

namespace {

inline GLuint GetGlShaderType(Resources::Shader::iShaderCodeLoader::ShaderType type) {
    using ShaderType = Resources::Shader::iShaderCodeLoader::ShaderType;
    switch (type) {
    case ShaderType::Fragment:
        return GL_FRAGMENT_SHADER;
    case ShaderType::Vertex:
        return GL_VERTEX_SHADER;
    case ShaderType::Geometry:
        return GL_GEOMETRY_SHADER;
    }
    throw std::runtime_error("Invalid shader type enum value!");
}

} // namespace

void ConstructShaderCommand::Execute() const {
    Array<GLuint> loaded_shaders;
    loaded_shaders.fill(Device::InvalidShaderStageHandle);

    unsigned loaded_count = 0;
    auto DeleteShaders = [&loaded_count, &loaded_shaders] {
        if (loaded_count > 0) {
            for (auto i : loaded_shaders) {
                glDeleteShader(i);
            }
        }
    };

    bool success = true;

    for (auto &shader_type : iShaderCodeLoader::kShaderFiles) {
        auto index = static_cast<unsigned>(shader_type.shader_type);

        if (code_array[index] == nullptr) {
            continue;
        }

        GLuint shader = glCreateShader(GetGlShaderType(shader_type.shader_type));
        loaded_shaders[index] = shader;

        glShaderSource(shader, 1, (const GLchar **)&code_array[index], nullptr);
        glCompileShader(shader);

        GLint Result = GL_FALSE;
        int info_length = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &Result);
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &info_length);

        if (Result == GL_TRUE && info_length <= 0) {
            ++loaded_count;
            continue; //compiled ok
        }

        success = false;

        std::string error_message(info_length + 1, '\0');
        glGetShaderInfoLog(shader, info_length, NULL, &error_message[0]);
        AddLog(Error, fmt::format("Unable to compile {} shader for {}. Error message:\n{}",
                                  shader_type.name, shader_name, error_message));

        break;
    }

    if (!success) {
        AddLogf(Error, "Shader compilation failed!");
        DeleteShaders();
        return;
    }

    //attach all shaders
    // GLuint ProgramID = glCreateProgram();

    for (auto i : loaded_shaders) {
        if (i != Device::InvalidShaderStageHandle) {
            glAttachShader(handle, i);
        }
    }

    //link program
    glLinkProgram(handle);

    //check program for errors
    GLint Result = GL_FALSE;
    int info_length = 0;

    glGetProgramiv(handle, GL_LINK_STATUS, &Result);
    glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &info_length);

    if (info_length > 1) {
        std::string error_message(info_length + 1, '\0');
        glGetProgramInfoLog(handle, info_length, NULL, &error_message[0]);
        AddLog(Error, fmt::format("Unable to link {} program. Error message:\n{}", shader_name,
                                  &error_message[0]));
        AddLogf(Error, "Shader linking failed!");
        glDeleteProgram(handle);
        DeleteShaders();
        return;
    }

    DeleteShaders();
}

void QueryStandardUniformsCommand::Execute() const {
    assert(uniforms);
    using Uniform = Resources::ShaderVariables::Uniform;

    for (auto i = 0u; i < static_cast<size_t>(Uniform::kMaxValue); ++i) {
        auto name = Resources::ShaderVariables::GetUniformName(static_cast<Uniform>(i));
        if (name == nullptr) {
            continue;
        }
        auto loc = glGetUniformLocation(shader_handle, name);
        uniforms->uniform[i] = loc;
        if (loc == Device::kInvalidShaderUniformHandle) {
            AddLogf(Debug, "Unable to get location of parameter '%s' in shader %d", name,
                    shader_handle);
        }
    }
}

} // namespace MoonGlare::Renderer::Commands
