#pragma once

#include "../Configuration.h"
#include "../ModalObjects.h"
#include "BaseGenerationAllocator.h"

namespace MoonGlare::Memory {

template<	template<typename ... > typename ALLOCATOR,
			class HANDLE, 
			typename ... CHANNELS
		>
struct GenerationRandomAllocator : public BaseGenerationAllocator<HANDLE, CHANNELS...> {
	enum {
		GenerationChannel,
		FreeIndexQueue,
		FirstUserChannel,
	};
	
	using BaseClass = BaseGenerationAllocator<HANDLE, CHANNELS...>;
	using Index_t = typename BaseClass::Index_t;
	using Generation_t = typename BaseClass::Generation_t;
	using Handle_t = typename BaseClass::Handle_t;
	using ChannelsIntegerSequence = typename BaseClass::ChannelsIntegerSequence;	

	using Storage_t = ALLOCATOR<
				Generation_t,			// generation of handle ids elements
				std::atomic<Index_t>,	// queue of free handle ids
				CHANNELS...				// table
			>;

	template<size_t CHANNEL>
	using UserChannelType = std::tuple_element_t<CHANNEL + FirstUserChannel, typename Storage_t::RowTuple>;

	GenerationRandomAllocator() {
		ResetAllocation();
	}
	GenerationRandomAllocator(NoConstruct_t) {
	}
	~GenerationRandomAllocator() {
	}

	bool IsHandleValid(Handle_t h) const {
		if (h.GetIndex() == 0)
			return false;
		auto index = h.GetIndex();
		if (index >= m_Memory.ElementCount())
			return false;
		return h.GetGeneration() == GetGeneration(index);
	}

	Handle_t Allocate() {
		if (Reserve() <= 0)
			return Handle_t{};
		Index_t id = AllocID();
		Handle_t h;
		h.SetGeneration(GetGeneration(id));
		h.SetIndex(id);
		return h;
	}

	void Free(Handle_t h) {
		if (!IsHandleValid(h))
			return;
		ReleaseIndex(h.GetIndex());
	}

	void ReleaseIndex(Index_t index) {
		NextGeneration(index);
		ReleaseID(index);
	}

	size_t GetRawIndex(Handle_t h) const {
		if (!IsHandleValid(h))
			return size_t(~0);
		return h.GetIndex();
	}

	void Clear() { ResetAllocation(); }

	Handle_t GetHandleFromIndex(Index_t index) const {
		if (m_Allocated <= index)
			return Handle_t();

		Index_t HandleID = GetTable2HandleMapping(index);
		Handle_t h;
		h.SetGeneration(GetGeneration(HandleID));
		h.SetIndex(HandleID);
		return h;
	}

//	void MemZero(Handle_t h) {
//	}

	size_t Allocated() const { return m_Allocated; }
	size_t Reserve() const { return m_Memory.ElementCount() - Allocated() - 1; }//compensate unused 0 index
	size_t Capacity() const { return m_Memory.ElementCount(); }

	Storage_t& GetStorage() { return m_Memory; }
	const Storage_t& GetStorage() const { return m_Memory; }

	template<size_t CHANNEL>
	UserChannelType<CHANNEL>& GetChannelValue(Handle_t h) LIBSPACE_ASSERT_NOEXCEPT {
		LIBSPACE_ASSERT(IsHandleValid(h));
		auto idx = GetRawIndex(h);
		LIBSPACE_BOUNDARY_ASSERT(idx < m_Memory.ElementCount());
		return m_Memory.GetChannelValue<CHANNEL + FirstUserChannel>(idx);
	}

	template<size_t CHANNEL>
	void SetChannelValue(Handle_t h, const UserChannelType<CHANNEL> & value) LIBSPACE_ASSERT_NOEXCEPT {
		LIBSPACE_ASSERT(IsHandleValid(h));
		auto idx = GetRawIndex(h);
		LIBSPACE_BOUNDARY_ASSERT(idx < m_Memory.ElementCount());
		m_Memory.SetChannelValue<CHANNEL + FirstUserChannel>(idx) = value;
	}

	template<size_t CHANNEL>
	const UserChannelType<CHANNEL>& GetChannelValue(Handle_t h) const LIBSPACE_ASSERT_NOEXCEPT {
		LIBSPACE_ASSERT(IsHandleValid(h));
		auto idx = GetRawIndex(h);
		LIBSPACE_BOUNDARY_ASSERT(idx < m_Memory.ElementCount());
		return m_Memory.GetChannelValue<CHANNEL + FirstUserChannel>(idx);
	}
private:
	Generation_t NextGeneration(Index_t HandleID) { 
		auto &v = m_Memory.GetChannelValue<GenerationChannel>(HandleID);
		v = (v + 1) & this->GenerationMask();
		if (v == 0)
			v = 1;
		return v;
	}
	Generation_t GetGeneration(Index_t HandleID) const { 
		return (m_Memory.GetChannelValue<GenerationChannel>(HandleID)) & this->GenerationMask();
	}

	Index_t AllocID() {
		Index_t index = 0;
		while ((index = m_FreeQueueFirst.exchange(0)) == 0)
			if (Reserve() <= 0)
				return 0;
		m_FreeQueueFirst.store(m_Memory.GetChannelValue<FreeIndexQueue>(index));
		++m_Allocated;
		return index;
	}
	void ReleaseID(Index_t index) {
		NextGeneration(index);
		m_Memory.GetChannelValue<FreeIndexQueue>(m_FreeQueueLast.exchange(index)).store(index);
		--m_Allocated;
	}

	void AssertHandle(Handle_t h) LIBSPACE_ASSERT_NOEXCEPT {
		LIBSPACE_ASSERT(h.GetIndex() > 0);
		LIBSPACE_ASSERT(h.GetIndex() < m_Memory.ElementCount());
	}

	void ResetAllocation() {
		for (Index_t i = 0; i < m_Memory.ElementCount(); ++i) {
			m_Memory.GetChannelValue<FreeIndexQueue>(i).store(i + 1);
			m_Memory.SetChannelValue<GenerationChannel>(i, 1);
		}
		m_FreeQueueFirst = 1;
		m_FreeQueueLast = static_cast<Index_t>(m_Memory.ElementCount() - 1);
		m_Allocated = 0;
	}

	void Swap(Index_t A, Index_t B) {
		SwapChannels(A, B, ChannelsIntegerSequence());
	}
	void Move(Index_t Src, Index_t Dst) {
		MoveChannel(Src, Dst, ChannelsIntegerSequence());
	}
	void MemZero(Index_t Index) {
		MemZeroChannel(Index, ChannelsIntegerSequence());
	}

	template<size_t ... INDEXES>
	void SwapChannels(Index_t A, Index_t B, std::index_sequence<INDEXES...>) {
		LIBSPACE_ASSERT(A != B);
		int v[] = {
			(std::swap(m_Memory.GetChannelValue<INDEXES + FirstUserChannel>(A), m_Memory.GetChannelValue<INDEXES + FirstUserChannel>(B)), 1)...
		};
		(void)v;
	}

	template<size_t ... INDEXES>
	void MoveChannel(Index_t Src, Index_t Dst, std::index_sequence<INDEXES...>) {
		LIBSPACE_ASSERT(Src != Dst);
		int v[] = { 
			(m_Memory.GetChannelValue<INDEXES + FirstUserChannel>(Dst) = m_Memory.GetChannelValue<INDEXES + FirstUserChannel>(Src), 1)...
		};
		(void)v;
	}

	template<size_t ... INDEXES>
	void MemZeroChannel(Index_t Index, std::index_sequence<INDEXES...>) {
		int v[] = {
			(memset(m_Memory.GetChannelValue<INDEXES + FirstUserChannel>(Index), 0, sizeof(UserChannelType<INDEXES>)), 1)...
		};
		(void)v;
	}
	
	Storage_t m_Memory;
	std::atomic<Index_t> m_Allocated;
	std::atomic<Index_t> m_FreeQueueFirst;
	std::atomic<Index_t> m_FreeQueueLast;
};

}
