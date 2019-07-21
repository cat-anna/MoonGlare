#pragma once

#include "../Configuration.h"
#include "BaseGenerationAllocator.h"
#include "../ModalObjects.h"

#include <random>
#include <chrono>

namespace MoonGlare::Memory {

template<	template<typename ... > typename ALLOCATOR,
			class HANDLE
		>
struct GenerationRandomBuffer : public BaseGenerationAllocator<HANDLE> {
	enum {
		GenerationChannel,
		FreeIndexQueue,
		UserMapping,
	};
	
	using BaseClass = BaseGenerationAllocator<HANDLE>;
	using Index_t = typename BaseClass::Index_t;
	using Generation_t = typename BaseClass::Generation_t;
	using Handle_t = typename BaseClass::Handle_t;
	using ChannelsIntegerSequence = typename BaseClass::ChannelsIntegerSequence;	

	constexpr Index_t InvalidIndex() const { return Index_t(~0); }

	using Storage_t = ALLOCATOR<
				Generation_t,			// generation of handle ids elements
				std::atomic<Index_t>,	// queue of free handle ids
				Index_t					// user mapping
			>;

	GenerationRandomBuffer(NoConstruct_t) { }
	GenerationRandomBuffer() {
		Rebuild();
	}
	~GenerationRandomBuffer() { }

	void Rebuild() {
		std::mt19937 rnum(static_cast<unsigned>(std::chrono::high_resolution_clock::now().time_since_epoch().count()));
		for (Index_t i = 0; i < m_Memory.ElementCount(); ++i) {
			m_Memory.template GetChannelValue<FreeIndexQueue>(i).store(i + 1);
			m_Memory.template SetChannelValue<GenerationChannel>(i, static_cast<Generation_t>(rnum()));
			m_Memory.template SetChannelValue<UserMapping>(i, 0);
		}
		m_FreeQueueFirst = 0;
		m_FreeQueueLast = static_cast<Index_t>(m_Memory.ElementCount() - 1);
		m_Allocated = 0;
	}

	bool IsHandleValid(Handle_t h) const {
		if (h.GetIndex() >= m_Memory.ElementCount())
			return false;
		return h.GetGeneration() == this->GetGeneration(h.GetIndex());
	}

	bool Allocate(Handle_t &hout) {
		if (this->Reserve() <= 0)
			return false;
		Index_t id = this->AllocID();
		hout.SetGeneration(this->GetGeneration(id));
		hout.SetIndex(id);
		return true;
	}

	/** ATTENTION: returned handle will always be valid! */
	Handle_t GetHandleFromIndex(Index_t idx) const {
		Handle_t h;
		h.SetGeneration(this->GetGeneration(idx));
		h.SetIndex(idx);
		return h;
	}

	void Free(Handle_t h) {
		if (!this->IsHandleValid(h))
			return;
		ReleaseIndex(h.GetIndex());
	}

	void ReleaseIndex(Index_t index) {
		NextGeneration(index);
		ReleaseID(index);
	}

	size_t GetRawIndex(Handle_t h) const {
		if (!this->IsHandleValid(h))
			return size_t(0);
		return h.GetIndex();
	}

	void Clear() { this->ResetAllocation(); }

	size_t Allocated() const { return m_Allocated; }
	size_t Reserve() const { return m_Memory.ElementCount() - Allocated(); }//compensate unused 0 index
	size_t Capacity() const { return m_Memory.ElementCount(); }

	Storage_t& GetStorage() { return m_Memory; }
	const Storage_t& GetStorage() const { return m_Memory; }

	template<class T>
	bool SetMapping(Index_t idx, const T &value) {
		if (idx > this->Reserve())
			return false;
		m_Memory.template SetChannelValue<UserMapping>(idx, static_cast<Index_t>(value));
		return true;
	}

	template<class T>
	bool GetMapping(Index_t idx, T &value) {
		if (idx > this->Reserve())
			return false;
		value = static_cast<T>(m_Memory.template GetChannelValue<UserMapping>(idx));
		return true;
	}

	template<class T>
	bool SetMapping(Handle_t h, const T &value) {
		if (!this->IsHandleValid(h))
			return false;
		m_Memory.template SetChannelValue<UserMapping>(h.GetIndex(), static_cast<Index_t>(value));
		return true;
	}

	template<class T>
	bool GetMapping(Handle_t h, T &value) {
		if (!IsHandleValid(h))
			return false;
		value = static_cast<T>(m_Memory.template GetChannelValue<UserMapping>(h.GetIndex()));
		return true;
	}
private:
	Generation_t NextGeneration(Index_t HandleID) { 
		auto &v = m_Memory.template GetChannelValue<GenerationChannel>(HandleID);
		v = (v + 1) & this->GenerationMask();
		if (v == 0)
			v = 1;
		return v;
	}
	Generation_t GetGeneration(Index_t HandleID) const { 
		return (m_Memory.template GetChannelValue<GenerationChannel>(HandleID)) & this->GenerationMask();
	}

	Index_t AllocID() {
		Index_t index = 0;
		while ((index = m_FreeQueueFirst.exchange(InvalidIndex())) == this->InvalidIndex())
			if (this->Reserve() <= 0)
				return 0;
		m_FreeQueueFirst.store(m_Memory.template GetChannelValue<FreeIndexQueue>(index));
		++m_Allocated;
		return index;
	}
	void ReleaseID(Index_t index) {
		this->NextGeneration(index);
		m_Memory.template GetChannelValue<FreeIndexQueue>(m_FreeQueueLast.exchange(index)).store(index);
		--m_Allocated;
	}

	void AssertHandle(Handle_t h) LIBSPACE_ASSERT_NOEXCEPT {
		LIBSPACE_ASSERT(h.GetIndex() > 0);
		LIBSPACE_ASSERT(h.GetIndex() < m_Memory.ElementCount());
	}

	Storage_t m_Memory;
	std::atomic<Index_t> m_Allocated;
	std::atomic<Index_t> m_FreeQueueFirst;
	std::atomic<Index_t> m_FreeQueueLast;
};

}
