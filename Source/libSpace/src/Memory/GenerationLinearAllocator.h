#pragma once

#include "../Configuration.h"
#include "BaseGenerationAllocator.h"

namespace Space {
namespace Memory {

template<	template<typename ... > typename ALLOCATOR,
			class HANDLE, 
			typename ... CHANNELS
		>
struct GenerationLinearAllocator : public BaseGenerationAllocator<HANDLE, CHANNELS...> {
	enum {
		MappingHandle2Table,
		GenerationChannel,
		MappingTable2Handle,
		FreeIndexStack,
		FirstUserChannel,
	};
	
	using BaseClass = BaseGenerationAllocator<HANDLE, CHANNELS...>;
	using Index_t = typename BaseClass::Index_t;
	using Generation_t = typename BaseClass::Generation_t;
	using Handle_t = typename BaseClass::Handle_t;
	using ChannelsIntegerSequence = typename BaseClass::ChannelsIntegerSequence;

	using Storage_t = ALLOCATOR<
				Index_t,				// mapping handle id -> table index
				Generation_t,			// generation of handle ids elements
				Index_t,				// mapping handle id <- table index
				std::atomic<Index_t>,	// stack of free handle ids
				CHANNELS...				// table
			>;

	template<size_t CHANNEL>
	using UserChannelType = std::tuple_element_t<CHANNEL + FirstUserChannel, typename Storage_t::RowTuple>;

	GenerationLinearAllocator() : m_Allocated(0) {
		Rebuild();
	}
	~GenerationLinearAllocator() { }

	bool IsHandleValid(Handle_t h) const {
		if (h.GetIndex() == 0)
			return false;
		auto index = h.GetIndex();
		if (index >= m_Memory.ElementCount())
			return false;
		return h.GetGeneration() == GetGeneration(index);
	}

	Handle_t Allocate() {
		if (m_FreeStackPosition >= Capacity())
			return Handle_t{};
		Index_t HandleID = AllocHandleID();
		Index_t Index = AllocIndex();
		SetHandle2TableMapping(HandleID, Index);
		SetTable2HandleMapping(Index, HandleID);
		Handle_t h;
		h.SetGeneration(GetGeneration(HandleID));
		h.SetIndex(HandleID);
		return h;
	}

	void Free(Handle_t h) {
		if (!IsHandleValid(h))
			return;
		NextGeneration(h.GetIndex());
		ReleaseIndex(GetHandle2TableMapping(h.GetIndex()));
		ReleaseHandleID(h.GetIndex());
		SetHandle2TableMapping(h.GetIndex(), 0);
	}

	size_t GetRawIndex(Handle_t h) const {
		if (!IsHandleValid(h))
			return size_t(~0);
		auto index = h.GetIndex();
		return GetHandle2TableMapping(index);
	}

//	void MemZero(Handle_t h) {
//	}

	void Rebuild() {
		m_Memory.MemZero();
		for (size_t i = 0, j = m_Memory.ElementCount(); i < j; ++i) {
			m_Memory.GetChannelValue<FreeIndexStack>(i).store(static_cast<Index_t>(i));
			m_Memory.SetChannelValue<GenerationChannel>(static_cast<Index_t>(i), 1);
			m_Memory.SetChannelValue<MappingHandle2Table>(static_cast<Index_t>(i), 0);
			m_Memory.SetChannelValue<MappingTable2Handle>(static_cast<Index_t>(i), 0);
		}
		m_FreeStackPosition = 1;
		m_Allocated = 0; //ignore index 0
	}

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
	Index_t GetHandle2TableMapping(Index_t handleid) const { return m_Memory.GetChannelValue<MappingHandle2Table>(handleid); }
	void SetHandle2TableMapping(Index_t handleid, Index_t index) { return m_Memory.SetChannelValue<MappingHandle2Table>(handleid, index); }
	
	Index_t GetTable2HandleMapping(Index_t index) const { return m_Memory.GetChannelValue<MappingTable2Handle>(index); }
	void SetTable2HandleMapping(Index_t handleid, Index_t index) { return m_Memory.SetChannelValue<MappingTable2Handle>(index, handleid); }

	Generation_t NextGeneration(Index_t HandleID) { 
		auto &v = m_Memory.GetChannelValue<GenerationChannel>(HandleID);
		v = (v + 1) & this->GenerationMask();
		return v;
	}
	Generation_t GetGeneration(Index_t HandleID) const { 
		return (m_Memory.GetChannelValue<GenerationChannel>(HandleID)) & this->GenerationMask();
	}

	Index_t AllocHandleID() { return m_Memory.GetChannelValue<FreeIndexStack>(m_FreeStackPosition++); }
	void ReleaseHandleID(Index_t handleid) { m_Memory.GetChannelValue<FreeIndexStack>(--m_FreeStackPosition).store(handleid); }

	Index_t AllocIndex() { return ++m_Allocated; }
	void ReleaseIndex(Index_t index) { 
		if (m_Allocated == index) {
			--m_Allocated;
			return;
		}
		
		auto last = m_Allocated.load();
		--m_Allocated;

		Move(last, index);//copy last -> index

		auto m = GetTable2HandleMapping(last);

		SetHandle2TableMapping(m, index);
		SetTable2HandleMapping(m, index);
	}
	
	void AssertHandle(Handle_t h) LIBSPACE_ASSERT_NOEXCEPT {
		LIBSPACE_ASSERT(h.GetIndex() > 0);
		LIBSPACE_ASSERT(h.GetIndex() < m_Memory.ElementCount());
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
	std::atomic<Index_t> m_FreeStackPosition;
};

}
}
