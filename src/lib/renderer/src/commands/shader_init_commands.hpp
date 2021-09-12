#pragma once

#include "renderer/device_types.hpp"
#include "renderer/resources/shader_resource_interface.hpp"
#include "resources/shader/shader_loader_interface.hpp"
#include <array>
#include <cassert>
#include <glad/glad.h>

namespace MoonGlare::Renderer::Commands {

namespace detail {} // namespace detail

struct GenShaderBatchCommand {
    GLuint *out;
    GLint count;
    void Execute() const {
        assert(out);
        for (GLint i = 0; i < count; ++i) {
            out[i] = glCreateProgram();
        }
    }
};

struct ConstructShaderCommand {
    using iShaderCodeLoader = Resources::Shader::iShaderCodeLoader;
    using ShaderType = iShaderCodeLoader::ShaderType;
    using ShaderCode = iShaderCodeLoader::ShaderCode;

    static constexpr size_t kMaxShaderTypes = static_cast<size_t>(ShaderType::MaxValue);

    using ShaderCodeBuffer = const char *;

    template <typename T>
    using Array = std::array<T, kMaxShaderTypes>;

    Array<ShaderCodeBuffer> code_array;
    const char *shader_name;
    Device::ShaderHandle handle;

    void Reset() { code_array.fill(nullptr); }
    void Execute() const;
};

struct QueryStandardUniformsCommand {
    Device::ShaderHandle handle;
    Resources::ShaderVariables *uniforms;

    void Execute() const;
};

struct InitShaderSamplersCommand {
    Device::ShaderHandle handle;
    const char *shader_name;

    void Execute() const;
};

} // namespace MoonGlare::Renderer::Commands

#if 0

namespace MoonGlare::Renderer::Commands {

//---------------------------------------------------------------------------------------

struct detail::ReleaseShaderResourceArgument {
	Device::ShaderHandle *m_ShaderHandle;
	const char* m_ShaderName;
	static void Execute(const ReleaseShaderResourceArgument *arg) {
		if (*arg->m_ShaderHandle != Device::InvalidShaderHandle) {
			DebugLogf(Info, "Released shader %u[%s]", arg->m_ShaderHandle, arg->m_ShaderName);
			glDeleteProgram(*arg->m_ShaderHandle);
			*arg->m_ShaderHandle = Device::InvalidShaderHandle;
		}
	}
};
using ReleaseShaderResource = Commands::CommandTemplate<detail::ReleaseShaderResourceArgument>;

//---------------------------------------------------------------------------------------

}

#endif
