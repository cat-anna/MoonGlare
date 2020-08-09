#pragma once

#include <array>
#include <atomic>
#include <cassert>
#include <limits>
#include <type_traits>

namespace MoonGlare::Memory {

template <size_t SIZE, typename ITEMTYPE, typename INDEXTYPE> struct BitmapAllocatorBase {
    using Item_t = ITEMTYPE;
    using Index_t = INDEXTYPE;

    static constexpr Index_t Size = SIZE;
    static constexpr Index_t SlotByteSize = sizeof(Item_t);
    static constexpr Index_t BitsPerSlot = SlotByteSize * 8;
    static constexpr Index_t SlotCount = Size / BitsPerSlot;

    static constexpr Item_t SlotEmpty = static_cast<Item_t>(0);
    static constexpr Item_t SlotFull = ~SlotEmpty;

    static_assert((Size % BitsPerSlot) == 0, "Size is not multiple of Item type bitsize");
    static_assert(std::numeric_limits<Index_t>::max() > Size, "Invalid size!");

    static constexpr size_t Capacity() { return Size; }

    struct SlotAndBit {
        Index_t slot, bit;
    };
    static constexpr Index_t MakeIndex(Index_t slot, Index_t bit) { return slot * BitsPerSlot + bit; }
    static constexpr SlotAndBit SplitIndex(Index_t index) {
        return SlotAndBit{index / BitsPerSlot, index & (BitsPerSlot - 1)};
    }
    static constexpr Item_t MakeMask(Index_t bit) { return static_cast<Item_t>(1u) << bit; }
};

template <size_t SIZE, typename ITEMTYPE = uint32_t, typename INDEXTYPE = uint32_t>
struct LinearAtomicBitmapAllocator : public BitmapAllocatorBase<SIZE, ITEMTYPE, INDEXTYPE> {

    using AtomicSlot = std::atomic<Item_t>;
    using AtomicArray = std::array<AtomicSlot, SlotCount>;

    bool IsAllocated(Index_t index) const {
        auto slotbit = SplitIndex(index);
        auto mask = MakeMask(slotbit.bit);
        if ((m_AtomicArray[slotbit.slot].load() & mask) != 0) {
            return true;
        } else {
            return false;
        }
    }

    bool Allocate(Index_t &indexout) {
        while (m_Allocated < Capacity()) {
            for (Index_t slot = 0; slot < SlotCount; ++slot) {
                if (m_AtomicArray[slot].load() != SlotFull) {
                    for (Index_t bit = 0; bit < BitsPerSlot; ++bit) {
                        auto mask = MakeMask(bit);
                        if ((m_AtomicArray[slot].fetch_or(mask) & mask) == 0) {
                            // got a match
                            indexout = MakeIndex(slot, bit);
                            ++m_Allocated;
                            return true;
                        }
                    }
                }
            }
        }
        return false;
    }

    bool Release(Index_t index) {
        auto slotbit = SplitIndex(index);
        auto mask = MakeMask(slotbit.bit);
        if ((m_AtomicArray[slotbit.slot].fetch_and(~mask) & mask) == 0) {
            return false;
        } else {
            --m_Allocated;
            return true;
        }
    }

    void ClearAllocation() {
        for (auto &item : m_AtomicArray)
            item.store(SlotEmpty);
        m_Allocated = 0;
    }

    constexpr Index_t Allocated() const { return m_Allocated.load(); }

protected:
    std::atomic<Index_t> m_Allocated;
    AtomicArray m_AtomicArray;
};

} // namespace MoonGlare::Memory
