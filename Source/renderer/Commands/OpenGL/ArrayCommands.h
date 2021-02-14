#pragma once

#include "../CommandQueueBase.h"
#include "Common.h"

namespace MoonGlare::Renderer::Commands {

struct VAOBindArgument {
	Device::VAOHandle m_VAO;
	static void Execute(const VAOBindArgument *arg) {
		glBindVertexArray(arg->m_VAO);
	}
};
using VAOBind = CommandTemplate<VAOBindArgument>;

struct VAOBindResourceArgument {
	Device::VAOHandle *m_VAO;
	static void Execute(const VAOBindResourceArgument *arg) {
		glBindVertexArray(*arg->m_VAO);
	}
};
using VAOBindResource = CommandTemplate<VAOBindResourceArgument>;

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

struct VAODrawElementsArgument {
	GLenum m_ElementMode;
	unsigned m_NumIndices;
	unsigned m_IndexValueType;

	static void Execute(const VAODrawElementsArgument *arg) {
		glDrawElements(arg->m_ElementMode, arg->m_NumIndices, arg->m_IndexValueType, 0);
	}
};
using VAODrawElements = CommandTemplate<VAODrawElementsArgument>;

struct VAODrawArraysArgument {
    GLenum mode;
    GLint first;
    GLsizei count;

    static void Execute(const VAODrawArraysArgument *arg) {
        glDrawArrays(arg->mode, arg->first, arg->count);
    }
};
using VAODrawArrays = CommandTemplate<VAODrawArraysArgument>;

//---------------------------------------------------------------------------------------
//Buffer handling
//---------------------------------------------------------------------------------------

namespace detail {
	template<GLenum target>
	struct BufferHandling {
		static void Bind(Device::BufferHandle handle) {
			glBindBuffer(target, handle);
		}
		static void Data(GLenum mode, GLsizeiptr bytecount, const void* data){
			glBufferData(target, bytecount, data, mode);
		}
	};

	template<typename Handler>
	struct BindBufferArgument {
		Device::BufferHandle m_Handle;
		static void Execute(const BindBufferArgument *arg) {
			Handler::Bind(arg->m_Handle);
		}
	};

	template<typename Handler>
	struct BindBufferResourceArgument {
		Device::BufferHandle *m_Handle;
		static void Execute(const BindBufferResourceArgument *arg) {
			Handler::Bind(*arg->m_Handle);
		}
	};

	template<typename Handler, GLenum Mode>
	struct BufferDataArgument {
		GLsizeiptr m_ByteCount;
		const void *m_DataPtr;
		static void Execute(const BufferDataArgument *arg) {
			Handler::Data(Mode, arg->m_ByteCount, arg->m_DataPtr);
		}
	};

	template<GLboolean Normalized>
	struct BufferChannelArgument {
		GLuint m_Channel;
		GLint m_ElementSize;
		GLenum m_ElementType;

		static void Channel(GLuint Channel, GLint ElementSize, GLenum ElementType) {
			glEnableVertexAttribArray(Channel);
			glVertexAttribPointer(Channel, ElementSize, ElementType, Normalized, 0, 0);
		}

		static void Execute(const BufferChannelArgument *arg) {
			Channel(arg->m_Channel, arg->m_ElementSize, arg->m_ElementType);
		}
	};

	using BindArrayBufferArgument = BindBufferArgument<BufferHandling<GL_ARRAY_BUFFER>>;
	using BindArrayIndexBufferArgument = BindBufferArgument<BufferHandling<GL_ELEMENT_ARRAY_BUFFER>>;

	using BindArrayBufferResourceArgument = BindBufferResourceArgument<BufferHandling<GL_ARRAY_BUFFER>>;
	using BindArrayIndexBufferResourceArgument = BindBufferResourceArgument<BufferHandling<GL_ELEMENT_ARRAY_BUFFER>>;

	using ArrayBufferStaticDataArgument = BufferDataArgument<BufferHandling<GL_ARRAY_BUFFER>, GL_STATIC_DRAW>;
	using IndexArrayBufferStaticDataArgument = BufferDataArgument<BufferHandling<GL_ELEMENT_ARRAY_BUFFER>, GL_STATIC_DRAW>;

	using ArrayBufferDynamicDataArgument = BufferDataArgument<BufferHandling<GL_ARRAY_BUFFER>, GL_DYNAMIC_DRAW>;
	using IndexArrayBufferDynamicDataArgument = BufferDataArgument<BufferHandling<GL_ELEMENT_ARRAY_BUFFER>, GL_DYNAMIC_DRAW>;

}

using BindArrayBuffer = CommandTemplate<detail::BindArrayBufferArgument>;
using BindArrayIndexBuffer = CommandTemplate<detail::BindArrayIndexBufferArgument>;
using BindArrayBufferResource = CommandTemplate<detail::BindArrayBufferResourceArgument>;
using BindArrayIndexBufferResource = CommandTemplate<detail::BindArrayIndexBufferResourceArgument>;

using ArrayBufferStaticData = CommandTemplate<detail::ArrayBufferStaticDataArgument>;
using ArrayIndexBufferStaticData = CommandTemplate<detail::IndexArrayBufferStaticDataArgument>;

using ArrayBufferDynamicData = CommandTemplate<detail::ArrayBufferDynamicDataArgument>;
using ArrayIndexBufferDynamicData = CommandTemplate<detail::IndexArrayBufferDynamicDataArgument>;

using ArrayBufferChannel = CommandTemplate<detail::BufferChannelArgument<GL_FALSE>>;
using ArrayBufferNormalizedChannel = CommandTemplate<detail::BufferChannelArgument<GL_TRUE>>;

//---------------------------------------------------------------------------------------
//Buffer allocation
//---------------------------------------------------------------------------------------

namespace detail {
struct BufferAllocation {
	using Handle_t = Device::BufferHandle;
	static void Allocate(GLsizei count, Handle_t *out) {
		glGenBuffers(count, out);
	}
	static void Release(GLsizei count, Handle_t *out) {
		glDeleteBuffers(count, out);
	}
};
}

using BufferSingleAllocate = CommandTemplate<detail::SingleAllocate <detail::BufferAllocation> >;
using BufferSingleRelease = CommandTemplate<detail::SingleRelease <detail::BufferAllocation> >;
using BufferBulkAllocate = CommandTemplate<detail::BulkAllocate <detail::BufferAllocation> >;
using BufferBulkRelease = CommandTemplate<detail::BulkRelease <detail::BufferAllocation> >;

//---------------------------------------------------------------------------------------
//VAO allocation
//---------------------------------------------------------------------------------------

namespace detail {
struct VAOAllocation {
	using Handle_t = Device::VAOHandle;
	static void Allocate(GLsizei count, Handle_t *out) {
		glGenVertexArrays(count, out);
	}
	static void Release(GLsizei count, Handle_t *out) {
		glDeleteVertexArrays(count, out);
	}
};
}

using VAOSingleAllocate = CommandTemplate<detail::SingleAllocate <detail::VAOAllocation> >;
using VAOSingleRelease = CommandTemplate<detail::SingleRelease <detail::VAOAllocation> >;
using VAOBulkAllocate = CommandTemplate<detail::BulkAllocate <detail::VAOAllocation> >;
using VAOBulkRelease = CommandTemplate<detail::BulkRelease <detail::VAOAllocation> >;

} //namespace MoonGlare::Renderer::Commands
