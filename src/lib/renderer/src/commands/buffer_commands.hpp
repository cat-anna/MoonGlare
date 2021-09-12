#pragma once

#include "command_common.hpp"
#include "debugger_support.hpp"
#include "renderer/configuration/vao.hpp"
#include "renderer/device_types.hpp"
#include <cassert>
#include <glad/glad.h>

namespace MoonGlare::Renderer::Commands {

//---------------------------------------------------------------------------------------
// Buffer allocation
//---------------------------------------------------------------------------------------

namespace detail {
struct BufferAllocation {
    using HandleType = Device::BufferHandle;
    static void Allocate(GLsizei count, HandleType *out) { glGenBuffers(count, out); }
    static void Release(GLsizei count, HandleType *out) { glDeleteBuffers(count, out); }
};
} // namespace detail

using BufferSingleAllocate = detail::SingleAllocate<detail::BufferAllocation>;
using BufferSingleRelease = detail::SingleRelease<detail::BufferAllocation>;
using BufferBulkAllocate = detail::BulkAllocate<detail::BufferAllocation>;
using BufferBulkRelease = detail::BulkRelease<detail::BufferAllocation>;

//---------------------------------------------------------------------------------------
// Vao allocation
//---------------------------------------------------------------------------------------

namespace detail {
struct VaoAllocation {
    using HandleType = Device::VaoHandle;
    static void Allocate(GLsizei count, HandleType *out) { glGenVertexArrays(count, out); }
    static void Release(GLsizei count, HandleType *out) { glDeleteVertexArrays(count, out); }
};
} // namespace detail

using VaoSingleAllocate = detail::SingleAllocate<detail::VaoAllocation>;
using VaoSingleRelease = detail::SingleRelease<detail::VaoAllocation>;
using VaoBulkAllocate = detail::BulkAllocate<detail::VaoAllocation>;
using VaoBulkRelease = detail::BulkRelease<detail::VaoAllocation>;

//---------------------------------------------------------------------------------------
// Vao binding
//---------------------------------------------------------------------------------------

struct VaoBind {
    Device::VaoHandle handle;
    void Execute() const { glBindVertexArray(handle); }
};

struct VaoBindIndirect {
    Device::VaoHandle *handle;
    void Execute() const {
        assert(handle);
        glBindVertexArray(*handle);
    }
};

struct VaoRelease {
    void Execute() const { glBindVertexArray(Device::kInvalidVaoHandle); }
};

//---------------------------------------------------------------------------------------
// Buffer data feed
//---------------------------------------------------------------------------------------

namespace detail {

template <GLenum Target_t>
struct BufferHandling {
    static void Bind(Device::BufferHandle handle) { glBindBuffer(Target_t, handle); }
    static void Release() { glBindBuffer(Target_t, Device::kInvalidBufferHandle); }
    static void Data(GLenum mode, GLsizeiptr bytecount, const void *data) {
        glBufferData(Target_t, bytecount, data, mode);
    }
};

template <typename Handler_t>
struct BindBuffer {
    Device::BufferHandle handle;
    void Execute() const { Handler_t::Bind(handle); }
};

template <typename Handler_t>
struct BindBufferIndirect {
    Device::BufferHandle *handle;
    void Execute() const { Handler_t::Bind(*handle); }
};

template <typename Handler_t, GLenum Mode_t>
struct BufferData {
    size_t byte_count;
    const void *data_ptr;
    void Execute() const { Handler_t::Data(Mode_t, static_cast<GLsizeiptr>(byte_count), data_ptr); }
};

template <GLboolean Normalized_t>
struct BufferChannel {
    GLuint channel;
    GLint element_size;
    GLenum element_type;

    void Execute() const {
        glEnableVertexAttribArray(channel);
        glVertexAttribPointer(channel, element_size, element_type, Normalized_t, 0, 0);
    }
};

struct BufferDisableChannel {
    GLuint channel;
    void Execute() const { glDisableVertexAttribArray(channel); }
};

template <typename Handler_t>
struct BufferRelease {
    void Execute() const { Handler_t::Release(); }
};

using ArrayBuffer = BufferHandling<GL_ARRAY_BUFFER>;
using IndexBuffer = BufferHandling<GL_ELEMENT_ARRAY_BUFFER>;

} // namespace detail

using BindArrayBuffer = detail::BindBuffer<detail::ArrayBuffer>;
using BindArrayBufferIndirect = detail::BindBufferIndirect<detail::ArrayBuffer>;

// using ReleaseArrayBuffer = detail::BufferRelease<detail::ArrayBuffer>;
// using ReleaseArrayBufferIndirect = detail::BufferRelease<detail::ArrayBuffer>;

using BindArrayIndexBuffer = detail::BindBuffer<detail::IndexBuffer>;
using BindArrayIndexBufferIndirect = detail::BindBuffer<detail::IndexBuffer>;

using ArrayBufferStaticData = detail::BufferData<detail::ArrayBuffer, GL_STATIC_DRAW>;
using ArrayIndexBufferStaticData = detail::BufferData<detail::IndexBuffer, GL_STATIC_DRAW>;

using ArrayBufferDynamicData = detail::BufferData<detail::ArrayBuffer, GL_DYNAMIC_DRAW>;
using ArrayIndexBufferDynamicData = detail::BufferData<detail::IndexBuffer, GL_DYNAMIC_DRAW>;

using ArrayBufferChannel = detail::BufferChannel<GL_FALSE>;
using ArrayBufferNormalizedChannel = detail::BufferChannel<GL_TRUE>;

using ArrayBufferDisableChannel = detail::BufferDisableChannel;

//---------------------------------------------------------------------------------------
// Draw Vao
//---------------------------------------------------------------------------------------

struct VaoDrawArrays {
    GLenum mode;
    GLint first;
    GLsizei count;

    void Execute() const { glDrawArrays(mode, first, count); }
};

struct VaoDrawElements {
    GLenum mode;
    GLsizei count;
    GLenum index_value_type;

    void Execute() const { glDrawElements(mode, count, index_value_type, nullptr); }
};

} // namespace MoonGlare::Renderer::Commands

#if 0

#include "../CommandQueueBase.h"
#include "Common.h"

namespace MoonGlare::Renderer::Commands {

//---------------------------------------------------------------------------------------


using VaoDrawTriangles = VaoDrawTrianglesArgument;

struct VaoDrawTrianglesBaseArgument {
	unsigned m_NumIndices;
	unsigned m_IndexValueType;
	unsigned m_BaseIndex;

	static void Execute(const VaoDrawTrianglesBaseArgument *arg) {
		glDrawElements(GL_TRIANGLES, m_NumIndices, m_IndexValueType, reinterpret_cast<void*>(m_BaseIndex));
	}
};
using VaoDrawTrianglesBase = VaoDrawTrianglesBaseArgument;

struct VaoDrawTrianglesBaseVertexArgument {
	unsigned m_NumIndices;
	unsigned m_IndexValueType;
	unsigned m_BaseIndex;
	GLint m_BaseVertex;

	static void Execute(const VaoDrawTrianglesBaseVertexArgument *arg) {
		glDrawElementsBaseVertex(GL_TRIANGLES, m_NumIndices, m_IndexValueType, reinterpret_cast<void*>(m_BaseIndex), m_BaseVertex);
	}
};
using VaoDrawTrianglesBaseVertex = VaoDrawTrianglesBaseVertexArgument;

struct VaoDrawElementsArgument {
	GLenum m_ElementMode;
	unsigned m_NumIndices;
	unsigned m_IndexValueType;

	static void Execute(const VaoDrawElementsArgument *arg) {
		glDrawElements(m_ElementMode, m_NumIndices, m_IndexValueType, 0);
	}
};
using VaoDrawElements = VaoDrawElementsArgument;



} //namespace MoonGlare::Renderer::Commands

#endif
