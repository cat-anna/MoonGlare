#pragma once

#include "../Configuration.h"
#include "BaseGenerationAllocator.h"
#include "ModalObjects.h"

namespace MoonGlare::Memory {

/**

GenerationLinearBuffer

Container with two internal mapping tables:
handle id -> table index
table index -> handle id

It allows to ensure that table elements may be freely swapped between indexes
ensuring that table is allocated linearly and may ordered in some way
ordering is not done by this container nor storing data itself.

The container handles only generation and mapping allocation.

*/
template<	template<typename ... > typename ALLOCATOR,
	class HANDLE 
>
struct GenerationLinearBuffer : public BaseGenerationAllocator<HANDLE> {
	enum {
		MappingHandle2Table,
		GenerationChannel,
		MappingTable2Handle,
		FreeIndexStack
	};

	using BaseClass = BaseGenerationAllocator<HANDLE>;
	using Index_t = typename BaseClass::Index_t;
	using Generation_t = typename BaseClass::Generation_t;
	using Handle_t = typename BaseClass::Handle_t;
	using ChannelsIntegerSequence = typename BaseClass::ChannelsIntegerSequence;

	using Storage_t = ALLOCATOR<
		Index_t,				// mapping handle id -> table index
		Generation_t,			// generation of handle ids elements
		Index_t,				// mapping handle id <- table index
		std::atomic<Index_t>	// stack of free handle ids
	>;

	GenerationLinearBuffer() : m_Allocated(0) {
		Rebuild();
	}
	/** No construct constructor. Internal structure must be rebuild manually. */
	GenerationLinearBuffer(NoConstruct_t) : m_Allocated(0) { 
	}
	~GenerationLinearBuffer() {}

	/** Check whether handle is valid */
	bool IsHandleValid(Handle_t h) const {
//		if (h.GetIndex() == 0)
//			return false;
		auto index = h.GetIndex();
		if (index >= m_Memory.ElementCount())
			return false;
		return h.GetGeneration() == GetGeneration(index);
	}

	/**
		Allocates new table and handle element
		table element will be allocated at the back of table
		handle id may be random
	*/
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

	/**
		Frees handle if its table element is the last one
		returns false if table element was not last or handle was not valid
	*/
	bool Free(Handle_t h) {
		if (!IsHandleValid(h))
			return true;
		if (!ReleaseIndex(GetHandle2TableMapping(h.GetIndex())))
			return false;
		NextGeneration(h.GetIndex());
		ReleaseHandleID(h.GetIndex());
		SetHandle2TableMapping(h.GetIndex(), 0);
		return true;
	}

	bool ReleaseLastIndex() {
		if (m_Allocated == 0)
			return false;
		auto tid = m_Allocated - 1;
		if (!ReleaseIndex(tid))
			return false;
		auto hid = GetTable2HandleMapping(tid);
		NextGeneration(hid);
		ReleaseHandleID(hid);
		return true;
	}

	/**
		Obtain table index associated with handle
		returns false if handle was not valid
	*/
	bool GetTableIndex(Handle_t h, size_t &index) const {
		if (!IsHandleValid(h))
			return false;
		index = GetHandle2TableMapping(h.GetIndex());
		return true;
	}

	Handle_t GetHandleToTableIndex(Index_t index) const {
		if (m_Allocated <= index) 
			return Handle_t();
		
		Index_t HandleID = GetTable2HandleMapping(index);
		Handle_t h;
		h.SetGeneration(GetGeneration(HandleID));
		h.SetIndex(HandleID);
		return h;
	}

	/** Rebuilds internal structure of container -> container is empty */
	void Rebuild() {
		m_Memory.MemZero();
		for (size_t i = 0, j = m_Memory.ElementCount(); i < j; ++i) {
			m_Memory.GetChannelValue<FreeIndexStack>(i).store(static_cast<Index_t>(i));
			m_Memory.SetChannelValue<GenerationChannel>(static_cast<Index_t>(i), 1);
			m_Memory.SetChannelValue<MappingHandle2Table>(static_cast<Index_t>(i), 0);
			m_Memory.SetChannelValue<MappingTable2Handle>(static_cast<Index_t>(i), 0);
		}
		m_FreeStackPosition = 0;
		m_Allocated = 0; //ignore index 0
	}

	bool Swap(Handle_t h, Index_t tblidx) {
		if (!IsHandleValid(h) || tblidx > m_Allocated)
			return false;
		if (m_Allocated == 1 || h.GetIndex() == tblidx)
			return true; //corner cases, nothing to swap
		return SwapElements(h, tblidx);
	}

	bool SwapIndex(Index_t atid, Index_t btid) {
		if (!atid > m_Allocated || btid > m_Allocated)
			return false;
		if (atid == btid)
			return true; //corner cases, nothing to swap
		return SwapElements(atid, btid);
	}

	bool MoveToBack(Handle_t h) {
		if (m_Allocated == 0)
			return false;
		if (m_Allocated == 1)
			return true;
		return Swap(h, static_cast<Index_t>(m_Allocated) - 1);
	}

	bool MoveIndexToBack(Index_t idx) {
		if (m_Allocated == 0)
			return false;
		if (m_Allocated == 1)
			return true;
		return SwapIndex(idx, static_cast<Index_t>(m_Allocated) - 1);
	}

	/** Get count of currently allocated elements */
	size_t Allocated() const { return m_Allocated; }
	/** Get count of remain unused elements */
	size_t Reserve() const { return m_Memory.ElementCount() - Allocated() - 1; }//compensate unused 0 index
	/** Get capacity of container */
	size_t Capacity() const { return m_Memory.ElementCount(); }

	/** Get reference to internal storage */
	Storage_t& GetStorage() { return m_Memory; }
	/** Get const reference to internal storage */
	const Storage_t& GetStorage() const { return m_Memory; }
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

	Index_t AllocIndex() { return m_Allocated++; }
	bool ReleaseIndex(Index_t index) {
		if (m_Allocated - 1 == index) {
			--m_Allocated;
			return true;
		}

		return false;

		//auto last = m_Allocated.load();
		//--m_Allocated;
		//
		//Move(last, index);//copy last -> index
		//
		//auto m = GetTable2HandleMapping(last);
		//
		//SetHandle2TableMapping(m, index);
		//SetTable2HandleMapping(m, index);
	}

	/**
		Internal function for swapping element mappings.
		All data has to be valid.
		always returns true 
	*/
	bool SwapElements(Handle_t ah, Index_t btid) LIBSPACE_ASSERT_NOEXCEPT {
		AssertHandle(ah);
		LIBSPACE_ASSERT(btid < m_Memory.ElementCount());
		auto ahid = ah.GetIndex();
		auto atid = GetHandle2TableMapping(ahid);
		auto bhid = GetTable2HandleMapping(btid);

		//H2T[ahid] = btid;
		SetHandle2TableMapping(ahid, btid);
		//H2T[bhid] = atid;
		SetHandle2TableMapping(bhid, atid);
		//T2H[btid] = ahid;
		SetTable2HandleMapping(btid, ahid);
		//T2H[atid] = bhid;
		SetTable2HandleMapping(atid, bhid);
		return true;
	}
	bool SwapElements(Index_t atid, Index_t btid) LIBSPACE_ASSERT_NOEXCEPT {
		LIBSPACE_ASSERT(atid < m_Memory.ElementCount());
		LIBSPACE_ASSERT(btid < m_Memory.ElementCount());
		auto bhid = GetTable2HandleMapping(btid);
		auto ahid = GetTable2HandleMapping(atid);

		//H2T[ahid] = btid;
		SetHandle2TableMapping(ahid, btid);
		//H2T[bhid] = atid;
		SetHandle2TableMapping(bhid, atid);
		//T2H[btid] = ahid;
		SetTable2HandleMapping(btid, ahid);
		//T2H[atid] = bhid;
		SetTable2HandleMapping(atid, bhid);
		return true;
	}

	void AssertHandle(Handle_t h) LIBSPACE_ASSERT_NOEXCEPT {
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
	std::atomic<Index_t> m_Allocated;//Count of used table elements
	std::atomic<Index_t> m_FreeStackPosition;//pointer to first free handle id stack
};

}
