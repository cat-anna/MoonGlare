#pragma once

namespace MoonGlare::Renderer {

#include "Configuration.Renderer.h"

struct GLTextureAllocator {
	using Index_t = GLuint;

	static void Allocate(uint32_t n, Index_t* textures) {
		glGenTextures(static_cast<GLsizei>(n), textures);
	}
	static void Release(uint32_t n, Index_t* textures) {
		glDeleteTextures(static_cast<GLsizei>(n), textures);
	}
};

template<uint32_t SIZE, typename Allocator_t>
struct IndexBuffer {
	static constexpr uint32_t Capacity = SIZE;
	using Allocator = Allocator_t;
	using Index_t = typename Allocator_t::Index_t;

//	IndexBuffer() {
//		m_RemainCount = 0;
//	}

	bool Allocate(Index_t &out) {
		LOCK_MUTEX(m_Lock);
		if (m_RemainCount == 0)
			return false;
		--m_RemainCount;
		out = m_Array[m_RemainCount];
		return true;
	}
	bool Release(Index_t idx) {
		LOCK_MUTEX(m_Lock);
		if (m_RemainCount == Capacity)
			return false;
		++m_RemainCount;
		m_Array[m_RemainCount] = idx;
		return true;
	}

	bool Initialize() {
		Allocator::Allocate(Capacity, &m_Array[0]);
		m_RemainCount = Capacity;
		return true;
	}
	bool Finalize() {
		Allocator::Release(Capacity, &m_Array[0]);
		m_Array.fill(0);
		m_RemainCount = 0;
		return true;
	}
private:
	using Array = std::array<Index_t, Capacity>;

	std::mutex m_Lock;
	uint32_t m_RemainCount;
	Array m_Array;
};

using TextureIndexBuffer = IndexBuffer<Configuration::IndexBuffer::TextureBuffer, GLTextureAllocator>;

} //namespace MoonGlare::Renderer 
