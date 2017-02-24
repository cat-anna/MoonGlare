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
#ifdef  XMATH_H
struct ShaderSetUniformMatrix4Argument {
	ShaderUniformHandle m_Location;
	math::RawMat4 m_Matrix;
	static void Execute(const ShaderSetUniformMatrix4Argument *arg) {
		glUniformMatrix4fv(arg->m_Location, 1, GL_FALSE, (const float*)&arg->m_Matrix);
	}
};
using ShaderSetUniformMatrix4 = CommandTemplate<ShaderSetUniformMatrix4Argument>;

struct ShaderSetUniformVec4Argument {
	ShaderUniformHandle m_Location;
	math::RawVec4 m_Vec;
	static void Execute(const ShaderSetUniformVec4Argument *arg) {
		glUniform4fv(arg->m_Location, 1, &arg->m_Vec[0]);
	}
};
using ShaderSetUniformVec4 = CommandTemplate<ShaderSetUniformVec4Argument>;

struct ShaderSetUniformVec3Argument {
	ShaderUniformHandle m_Location;
	math::RawVec3 m_Vec;
	static void Execute(const ShaderSetUniformVec3Argument *arg) {
		glUniform3fv(arg->m_Location, 1, &arg->m_Vec[0]);
	}
};
using ShaderSetUniformVec3 = CommandTemplate<ShaderSetUniformVec3Argument>;

struct ShaderSetUniformVec2Argument {
	ShaderUniformHandle m_Location;
	float m_Vec[2];
	static void Execute(const ShaderSetUniformVec2Argument *arg) {
		glUniform2fv(arg->m_Location, 1, arg->m_Vec);
	}
};
using ShaderSetUniformVec2 = CommandTemplate<ShaderSetUniformVec2Argument>;

struct ShaderSetUniformIVec2Argument {
	ShaderUniformHandle m_Location;
	int m_Vec[2];
	static void Execute(const ShaderSetUniformIVec2Argument *arg) {
		glUniform2iv(arg->m_Location, 1, arg->m_Vec);
	}
};
using ShaderSetUniformIVec2 = CommandTemplate<ShaderSetUniformIVec2Argument>;

struct ShaderSetUniformFloatArgument {
	ShaderUniformHandle m_Location;
	float m_Float;
	static void Execute(const ShaderSetUniformFloatArgument *arg) {
		glUniform1f(arg->m_Location, arg->m_Float);
	}
};
using ShaderSetUniformFloat = CommandTemplate<ShaderSetUniformFloatArgument>;
#endif
//---------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------

namespace detail {

struct ShaderResourceBindArgument {
	ShaderHandle *m_Shader;
	static void Execute(const ShaderResourceBindArgument *arg) {
		glUseProgram(*arg->m_Shader);
	}
};

struct ShaderResourcSetUniformArgumentBase {
	static void Set(ShaderUniformHandle h, const emath::fmat4 &matrix) {
		glUniformMatrix4fv(h, 1, GL_FALSE, (const float*)&matrix);
	}
	static void Set(ShaderUniformHandle h, const emath::fvec4 &v) {
		glUniform4fv(h, 1, (const float*)&v);
	}
	static void Set(ShaderUniformHandle h, const emath::fvec3 &v) {
		glUniform3fv(h, 1, (const float*)&v);
	}
	static void Set(ShaderUniformHandle h, const emath::fvec2 &v) {
		glUniform2fv(h, 1, (const float*)&v);
	}
};

template<typename T>
struct ShaderResourcSetUniformArgument {
	ShaderUniformHandle* m_Handle;
	T m_Value;
	static void Execute(const ShaderResourcSetUniformArgument *arg) {
		ShaderResourcSetUniformArgumentBase::Set(*arg->m_Handle, arg->m_Value);
	}
};

}
using ShaderResourceBind = CommandTemplate<detail::ShaderResourceBindArgument>;

template<typename T>
using ShaderResourcSetUniform = CommandTemplate<detail::ShaderResourcSetUniformArgument<T>>;


} //namespace MoonGlare::Renderer::Commands
