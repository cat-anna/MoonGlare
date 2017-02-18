#pragma once

#include "../CommandQueueBase.h"
#include "Common.h"

namespace MoonGlare::Renderer::Commands{

struct Texture2DBindArgument {
	TextureHandle m_Texture;
	static void Execute(const Texture2DBindArgument *arg) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, arg->m_Texture);
	}
};
using Texture2DBind = CommandTemplate<Texture2DBindArgument>;

struct Texture2DBindUnitArgument {
	TextureHandle m_Texture;
	unsigned m_UnitIndex;
	static void Execute(const Texture2DBindUnitArgument *arg) {
		glActiveTexture(GL_TEXTURE0 + arg->m_UnitIndex);
		glBindTexture(GL_TEXTURE_2D, arg->m_Texture);
	}
};
using Texture2DBindUnit = CommandTemplate<Texture2DBindUnitArgument>;

//---------------------------------------------------------------------------------------

struct Texture2DResourceBindArgument : public TextureCommandBase {
	static void Execute(const Texture2DResourceBindArgument *arg) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, arg->m_HandleArray[arg->m_Handle.m_Index]);
	}
};
using Texture2DResourceBind = CommandTemplate<Texture2DResourceBindArgument>;

struct Texture2DResourceBindUnitArgument : public TextureCommandBase {
	uint16_t m_UnitIndex;
	static void Execute(const Texture2DResourceBindUnitArgument *arg) {
		glActiveTexture(GL_TEXTURE0 + arg->m_UnitIndex);
		glBindTexture(GL_TEXTURE_2D, arg->m_HandleArray[arg->m_Handle.m_Index]);
	}
};
using Texture2DResourceBindUnit = CommandTemplate<Texture2DResourceBindUnitArgument>;

//---------------------------------------------------------------------------------------

namespace detail {
struct TextureAllocation {
	using Handle_t = TextureHandle;
	static void Allocate(GLsizei count, Handle_t *out) {
		glGenTextures(count, out);
	}
	static void Release(GLsizei count, Handle_t *out) {
		glDeleteTextures(count, out);
	}
};
}

using TextureSingleAllocate = CommandTemplate<detail::SingleAllocate <detail::TextureAllocation> >;
using TextureSingleRelease = CommandTemplate<detail::SingleRelease <detail::TextureAllocation> >;
using TextureBulkAllocate = CommandTemplate<detail::BulkAllocate <detail::TextureAllocation> >;
using TextureBulkRelease = CommandTemplate<detail::BulkRelease <detail::TextureAllocation> >;

//---------------------------------------------------------------------------------------

struct Texture2DImageArgument {
	GLint m_InternalFormat;
	GLsizei m_Size[2];
	GLenum m_Format;
	GLenum m_Type;
	void *m_Pixels;
	static void Execute(const Texture2DImageArgument *arg) {
		glTexImage2D(GL_TEXTURE_2D, 0, arg->m_InternalFormat, arg->m_Size[0], arg->m_Size[1], 0, arg->m_Format, arg->m_Type, arg->m_Pixels);
	}
};
using Texture2DImage = CommandTemplate<Texture2DImageArgument>;

//---------------------------------------------------------------------------------------

template<typename ArgType>
struct Texture2DParameterArgument {
	GLenum m_Parameter;
	ArgType m_Value;
	static void Execute(const Texture2DParameterArgument *arg) {
		Set(arg->m_Parameter, arg->m_Value);
	}

	static void Set(GLenum Parameter, GLint Value) {
		glTexParameteri(GL_TEXTURE_2D, Parameter, Value);
	}
	static void Set(GLenum Parameter, GLfloat Value) {
		glTexParameterf(GL_TEXTURE_2D, Parameter, Value);
	}

	static void Set(GLenum Parameter, const GLint *Value) {
		glTexParameteriv(GL_TEXTURE_2D, Parameter, Value);
	}
	static void Set(GLenum Parameter, const GLfloat *Value) {
		glTexParameterfv(GL_TEXTURE_2D, Parameter, Value);
	}
};

template<typename ArgType>
using Texture2DParameter = CommandTemplate<Texture2DParameterArgument<ArgType>>;

using Texture2DParameterInt = Texture2DParameter<GLint>;
using Texture2DParameterFloat  = Texture2DParameter<GLfloat>;

} //namespace MoonGlare::Renderer::Commands
