#pragma once

#include "../../Resources/AssetLoaderInterface.h"
#include "../../Configuration.Renderer.h"
#include "../CommandQueueBase.h"
#include "Common.h"

namespace MoonGlare::Renderer::Commands {

namespace detail {
	struct GetShaderUnfiormsArgument;
	struct InitShaderSamplersArgument;
	struct ReleaseShaderResourceArgument;
	struct ConstructShaderArgument;
}

//---------------------------------------------------------------------------------------

struct detail::ReleaseShaderResourceArgument {
	Device::ShaderHandle *m_ShaderHandle;
	const char* m_ShaderName;
	static void Execute(const ReleaseShaderResourceArgument *arg) {
		if (*arg->m_ShaderHandle != Device::InvalidShaderHandle) {
			DebugLogf(Info, "Released shader %u[%s]", arg->m_ShaderHandle, arg->m_ShaderName);
			glDeleteProgram(*arg->m_ShaderHandle);
			*arg->m_ShaderHandle == Device::InvalidShaderHandle;
		}
	}
};
using ReleaseShaderResource = Commands::CommandTemplate<detail::ReleaseShaderResourceArgument>;

//---------------------------------------------------------------------------------------

struct detail::ConstructShaderArgument {
	using ShaderCode = Resources::ShaderCodeLoader::ShaderCode;
	using ShaderType = Resources::ShaderCodeLoader::ShaderType;

	static constexpr size_t MaxShaderLines = 8;
	static constexpr size_t MaxShaderTypes = static_cast<size_t>(ShaderType::MaxValue);
	using ShaderCodeBuffer = std::array<const char *, MaxShaderLines>;

	template<typename T>
	using Array = std::array<T, MaxShaderTypes>;

	Array<bool> m_Valid;
	Array<ShaderCodeBuffer> m_CodeArray;
	const char* m_ShaderName;
	Device::ShaderHandle *m_ShaderOutput;

	struct ShaderTypeInfo {
		ShaderType m_Type;
		GLuint m_GLID;
		const char *m_Name;
	};
	static constexpr std::array<ShaderTypeInfo, MaxShaderTypes> ShaderTypes = {
		ShaderTypeInfo{ ShaderType::Vertex, GL_VERTEX_SHADER, "vertex", },
		ShaderTypeInfo{ ShaderType::Fragment, GL_FRAGMENT_SHADER, "fragment", },
		ShaderTypeInfo{ ShaderType::Geometry, GL_GEOMETRY_SHADER, "geometry", },
	};

	void Run() const {
		std::array<GLuint, MaxShaderTypes> LoadedShaders;
		LoadedShaders.fill(Device::InvalidShaderStageHandle);

		unsigned LoadedCount = 0;
		auto DeleteShaders = [&LoadedCount, &LoadedShaders] {
			if (LoadedCount > 0) {
				for (auto i : LoadedShaders)
					glDeleteShader(i);
			}
		};

		bool Success = true;

		for (auto &shadertype : ShaderTypes) {
			auto index = static_cast<unsigned>(shadertype.m_Type);

			if (!m_Valid[index])
				continue;

			GLuint shader = glCreateShader(shadertype.m_GLID);
			LoadedShaders[index] = shader;

			glShaderSource(shader, m_CodeArray[index].size(), (const GLchar**)&m_CodeArray[index][0], NULL);	//TODO: check what is last argument
			glCompileShader(shader);

			GLint Result = GL_FALSE;
			int InfoLogLength = 0;
			glGetShaderiv(shader, GL_COMPILE_STATUS, &Result);
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &InfoLogLength);

			if (InfoLogLength <= 0) {
				++LoadedCount;
				continue; //compiled ok
			}

			Success = false;

			std::string ShaderErrorMessage(InfoLogLength + 1, '\0');
			glGetShaderInfoLog(shader, InfoLogLength, NULL, &ShaderErrorMessage[0]);
			AddLogf(Error, "Unable to compile %s shader for %s. Error message:\n%s",
				shadertype.m_Name, m_ShaderName, ShaderErrorMessage.c_str());

			break;
		}

		if (!Success) {
			AddLogf(Error, "Shader compilation failed!");
			DeleteShaders();
			return;
		}

		//attach all shaders
		GLuint ProgramID = glCreateProgram();

		for (auto i : LoadedShaders)
			if (i != Device::InvalidShaderStageHandle)
				glAttachShader(ProgramID, i);

		//link program
		glLinkProgram(ProgramID);

		//check program for errors
		GLint Result = GL_FALSE;
		int InfoLogLength = 0;

		glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
		glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);

		if (InfoLogLength > 1) {
			std::string ProgramErrorMessage(InfoLogLength + 1, '\0');
			glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
			AddLogf(Error, "Unable to link %s program. Error message:\n%s", m_ShaderName, &ProgramErrorMessage[0]);
			AddLogf(Error, "Shader linking failed!");
			glDeleteProgram(ProgramID);
			DeleteShaders();
			return;
		}

		DeleteShaders();
		*m_ShaderOutput = ProgramID;
	}

	static void Execute(const ConstructShaderArgument *arg) {
		return arg->Run();
	}
};
using ConstructShader = Commands::CommandTemplate<detail::ConstructShaderArgument>;

//---------------------------------------------------------------------------------------

struct detail::GetShaderUnfiormsArgument {
	const char **m_Names;
	unsigned m_Count;
	Device::ShaderHandle *m_ShaderHandle;
	Configuration::Shader::UniformLocations *m_Locations;
	const char *m_ShaderName;

	void Run() const {
		if (*m_ShaderHandle == Device::InvalidShaderHandle)
			return;
		glUseProgram(*m_ShaderHandle);
		for (auto i = 0u; i < m_Count; ++i) {
			if (!m_Names[i])
				//uniform is not named, just skip
				continue;
			auto loc = glGetUniformLocation(*m_ShaderHandle, m_Names[i]);
			(*m_Locations)[i] = loc;
			if (loc == Device::InvalidShaderUniformHandle) {
				DebugLogf(Warning, "Unable to get location of parameter '%s' in shader '%s'", m_Names[i], m_ShaderName);
			} else {
				//DebugLogf(Hint, "Shader uniform location %s.%s -> %u", m_ShaderName, m_Names[i], loc);
			}
		}
	}
	static void Execute(const GetShaderUnfiormsArgument *arg) {
		arg->Run();
	}
};
using GetShaderUnfiorms = Commands::CommandTemplate<detail::GetShaderUnfiormsArgument>;

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
