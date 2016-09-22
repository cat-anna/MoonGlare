#pragma once
#ifndef GLARRAYCOMMANDS_H_
#define GLARRAYCOMMANDS_H_

#include "../OpenGL/nfOpenGL.h"

namespace MoonGlare {
namespace Renderer {
namespace OpenGL {
namespace Commands {

struct VAOBindArgument {
	VAOHandle m_VAO;
	static void Execute(const VAOBindArgument *arg) {
		glBindVertexArray(arg->m_VAO);
	}
};
using VAOBind = CommandTemplate<VAOBindArgument>;

struct VAOReleaseArgument {
	static void Execute(const VAOReleaseArgument *arg) {
		glBindVertexArray(0);
	}
};
using VAORelease = CommandTemplate<VAOReleaseArgument>;

//---------------------------------------------------------------------------------------

struct VAODrawTrianglesArgument {
	unsigned m_NumIndices;
	unsigned m_IndexValueType;

	static void Execute(const VAODrawTrianglesArgument *arg) {
		glDrawElements(GL_TRIANGLES, arg->m_NumIndices, arg->m_IndexValueType, 0);
	}
};
using VAODrawTriangles = CommandTemplate<VAODrawTrianglesArgument>;

} //namespace Commands
} //namespace OpenGL
} //namespace Renderer 
} //namespace MoonGlare 

#endif // NFRENDERER_H_
