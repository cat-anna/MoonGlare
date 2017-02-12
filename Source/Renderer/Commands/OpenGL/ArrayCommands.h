#pragma once

#include "../CommandQueueBase.h"

namespace MoonGlare::Renderer::Commands{

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

struct BindArrayBufferArgument {
	GLuint m_Buffer;

	static void Execute(const BindArrayBufferArgument *arg) {
		glBindBuffer(GL_ARRAY_BUFFER, arg->m_Buffer);
	}
};
using  BindArrayBuffer = CommandTemplate<BindArrayBufferArgument>;

struct ArrayBufferStaticDataArgument {
	GLsizeiptr m_ByteCount;
	void *m_DataPtr;

	static void Execute(const ArrayBufferStaticDataArgument *arg) {
		glBufferData(GL_ARRAY_BUFFER, arg->m_ByteCount, arg->m_DataPtr, GL_STATIC_DRAW);
	}
};
using ArrayBufferStaticData = CommandTemplate<ArrayBufferStaticDataArgument>;

struct ArrayBufferDynamicDataArgument {
	GLsizeiptr m_ByteCount;
	void *m_DataPtr;

	static void Execute(const ArrayBufferDynamicDataArgument *arg) {
		glBufferData(GL_ARRAY_BUFFER, arg->m_ByteCount, arg->m_DataPtr, GL_STATIC_DRAW);
	}
};
using ArrayBufferDynamicData = CommandTemplate<ArrayBufferDynamicDataArgument>;

//---------------------------------------------------------------------------------------

struct VAODrawTrianglesArgument {
	unsigned m_NumIndices;
	unsigned m_IndexValueType;

	static void Execute(const VAODrawTrianglesArgument *arg) {
		glDrawElements(GL_TRIANGLES, arg->m_NumIndices, arg->m_IndexValueType, 0);
	}
};
using VAODrawTriangles = CommandTemplate<VAODrawTrianglesArgument>;

struct VAODrawTrianglesBaseArgument {
	unsigned m_NumIndices;
	unsigned m_IndexValueType;
	unsigned m_BaseIndex;

	static void Execute(const VAODrawTrianglesBaseArgument *arg) {
		glDrawElements(GL_TRIANGLES, arg->m_NumIndices, arg->m_IndexValueType, reinterpret_cast<void*>(arg->m_BaseIndex));
	}
};
using VAODrawTrianglesBase = CommandTemplate<VAODrawTrianglesBaseArgument>;

struct VAODrawTrianglesBaseVertexArgument {
	unsigned m_NumIndices;
	unsigned m_IndexValueType;
	unsigned m_BaseIndex;
	GLint m_BaseVertex;

	static void Execute(const VAODrawTrianglesBaseVertexArgument *arg) {
		glDrawElementsBaseVertex(GL_TRIANGLES, arg->m_NumIndices, arg->m_IndexValueType, reinterpret_cast<void*>(arg->m_BaseIndex), arg->m_BaseVertex);
	}
};
using VAODrawTrianglesBaseVertex = CommandTemplate<VAODrawTrianglesBaseVertexArgument>;

} //namespace MoonGlare::Renderer::Commands
