#pragma once

#include "renderer/device_types.hpp"
#include "renderer/resources/shader_resource_interface.hpp"
#include "resources/shader/shader_loader_interface.hpp"
#include <array>
#include <glad/glad.h>

namespace MoonGlare::Renderer::Commands {

namespace detail {} // namespace detail

struct GenShaderBatchCommand {
    GLuint *base;
    size_t count;
    void Execute() const {
        if (base != nullptr) {
            for (size_t i = 0; i < count; ++i) {
                base[i] = glCreateProgram();
            }
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
    Device::ShaderHandle shader_handle;
    Resources::ShaderVariables *uniforms;

    void Execute() const;
};

} // namespace MoonGlare::Renderer::Commands

#if 0

#include "../../Configuration.Renderer.h"
#include "../CommandQueueBase.h"
#include "Common.h"
#include <Renderer/Resources/Shader/ShaderCodeLoader.h>

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

struct detail::InitShaderSamplersArgument {
	const char **m_Names;
	unsigned m_Count;
	Device::ShaderHandle *m_ShaderHandle;
	const char *m_ShaderName;

	void Run() const {
		if (*m_ShaderHandle == Device::InvalidShaderHandle)
			return;

		glUseProgram(*m_ShaderHandle);
		for (auto i = 0u; i < m_Count; ++i) {
			auto name = m_Names[i];
			if (!name)
				//sampler is not named, just skip
				continue;
			auto loc = glGetUniformLocation(*m_ShaderHandle, name);
			if (loc != Device::InvalidShaderUniformHandle) {
				glUniform1i(loc, static_cast<GLint>(i));
			}
			else {
				DebugLogf(Warning, "Unable to get location of sampler '%s' in shader '%s'", name, m_ShaderName);
			}
		}
	}
	static void Execute(const InitShaderSamplersArgument *arg) {
		arg->Run();
	}
};
using InitShaderSamplers = Commands::CommandTemplate<detail::InitShaderSamplersArgument>;

//---------------------------------------------------------------------------------------


}

#endif
