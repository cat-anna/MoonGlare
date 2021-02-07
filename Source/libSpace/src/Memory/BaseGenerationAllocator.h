#pragma once

#include "../Configuration.h"

namespace Space {
namespace Memory {

template<	class HANDLE, 
			typename ... CHANNELS
		>
struct BaseGenerationAllocator {

	using Handle_t = HANDLE;
	using Index_t = typename Handle_t::Index_t;
	using Generation_t = typename Handle_t::Generation_t;

	constexpr static Generation_t GenerationBits() { return Handle_t::GenerationBits   ; }
	constexpr static Generation_t GenerationMask() { return (1 << GenerationBits()) - 1  ; }

	using ChannelsIntegerSequence = std::make_index_sequence<sizeof...(CHANNELS)>;

protected:
	BaseGenerationAllocator() {}
};

}
}
