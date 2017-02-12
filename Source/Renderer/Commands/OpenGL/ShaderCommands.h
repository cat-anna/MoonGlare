#pragma once

#include "../CommandQueueBase.h"

namespace MoonGlare::Renderer::Commands {

struct ShaderBindArgument {
	ShaderHandle m_Shader;
	static void Execute(const ShaderBindArgument *arg) {
		glUseProgram(arg->m_Shader);
	}
};
using ShaderBind = CommandTemplate<ShaderBindArgument>;

//---------------------------------------------------------------------------------------

struct ShaderSetUniformMatrix4Argument {
	ShadeUniformLocation m_Location;
	math::RawMat4 m_Matrix;
	static void Execute(const ShaderSetUniformMatrix4Argument *arg) {
		glUniformMatrix4fv(arg->m_Location, 1, GL_FALSE, (const float*)&arg->m_Matrix);
	}
};
using ShaderSetUniformMatrix4 = CommandTemplate<ShaderSetUniformMatrix4Argument>;

struct ShaderSetUniformVec4Argument {
	ShadeUniformLocation m_Location;
	math::RawVec4 m_Vec;
	static void Execute(const ShaderSetUniformVec4Argument *arg) {
		glUniform4fv(arg->m_Location, 1, &arg->m_Vec[0]);
	}
};
using ShaderSetUniformVec4 = CommandTemplate<ShaderSetUniformVec4Argument>;

struct ShaderSetUniformVec3Argument {
	ShadeUniformLocation m_Location;
	math::RawVec3 m_Vec;
	static void Execute(const ShaderSetUniformVec3Argument *arg) {
		glUniform3fv(arg->m_Location, 1, &arg->m_Vec[0]);
	}
};
using ShaderSetUniformVec3 = CommandTemplate<ShaderSetUniformVec3Argument>;

struct ShaderSetUniformVec2Argument {
	ShadeUniformLocation m_Location;
	float m_Vec[2];
	static void Execute(const ShaderSetUniformVec2Argument *arg) {
		glUniform2fv(arg->m_Location, 1, arg->m_Vec);
	}
};
using ShaderSetUniformVec2 = CommandTemplate<ShaderSetUniformVec2Argument>;

struct ShaderSetUniformIVec2Argument {
	ShadeUniformLocation m_Location;
	int m_Vec[2];
	static void Execute(const ShaderSetUniformIVec2Argument *arg) {
		glUniform2iv(arg->m_Location, 1, arg->m_Vec);
	}
};
using ShaderSetUniformIVec2 = CommandTemplate<ShaderSetUniformIVec2Argument>;

struct ShaderSetUniformFloatArgument {
	ShadeUniformLocation m_Location;
	float m_Float;
	static void Execute(const ShaderSetUniformFloatArgument *arg) {
		glUniform1f(arg->m_Location, arg->m_Float);
	}
};
using ShaderSetUniformFloat = CommandTemplate<ShaderSetUniformFloatArgument>;

} //namespace MoonGlare::Renderer::Commands
