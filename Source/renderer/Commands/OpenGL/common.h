#pragma once
namespace MoonGlare::Renderer::Commands {

namespace detail {

template<typename Allocator>
struct SingleAllocate {
	using Handle_t = typename Allocator::Handle_t;
	Handle_t *m_Out;
	static void Execute(const SingleAllocate *arg) {
		Allocator::Allocate(1, arg->m_Out);
	}
};
template<typename Allocator>
struct SingleRelease {
	using Handle_t = typename Allocator::Handle_t;
	Handle_t *m_Out;
	static void Execute(const SingleRelease *arg) {
		Allocator::Release(1, arg->m_Out);
	}
};

template<typename Allocator>
struct BulkAllocate {
	using Handle_t = typename Allocator::Handle_t;
	Handle_t *m_OutPtr;
	GLsizei m_Count;
	static void Execute(const BulkAllocate *arg) {
		Allocator::Allocate(arg->m_Count, arg->m_OutPtr);
	}
};
template<typename Allocator>
struct BulkRelease {
	using Handle_t = typename Allocator::Handle_t;
	Handle_t *m_OutPtr;
	GLsizei m_Count;
	static void Execute(const BulkRelease *arg) {
		Allocator::Release(arg->m_Count, arg->m_OutPtr);
	}
};

}

}