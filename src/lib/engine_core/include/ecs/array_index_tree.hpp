#pragma once

#include "pointer_memory_array.hpp"
#include <array>
#include <boost/dynamic_bitset.hpp>
#include <boost/tti/has_member_function.hpp>
#include <cassert>
#include <fstream>
#include <iostream>
#include <string>
#include <type_traits>
#include <vector>

namespace MoonGlare::ECS {
namespace detail {
BOOST_TTI_HAS_MEMBER_FUNCTION(ReleaseElement)
BOOST_TTI_HAS_MEMBER_FUNCTION(InitElement)
BOOST_TTI_HAS_MEMBER_FUNCTION(SwapValues)
BOOST_TTI_HAS_MEMBER_FUNCTION(GetElementName)
BOOST_TTI_HAS_MEMBER_FUNCTION(ClearArrays)

template <typename T, typename IsEnumType>
struct ArrayIndexTreeIndexType {};
template <typename T>
struct ArrayIndexTreeIndexType<T, std::true_type> {
    using IntType = std::underlying_type_t<T>;
};
template <typename T>
struct ArrayIndexTreeIndexType<T, std::false_type> {
    using IntType = T;
};

struct ArrayIndexTreeCallback {};

template <typename Vector, typename Starter>
Vector Vectorize(Starter st) {
    Vector r;
    for (auto idx : st) {
        r.emplace_back(idx);
    }
    return r;
}

template <typename ElementIndex_t, typename Indexer_t, typename Callback_t = detail::ArrayIndexTreeCallback>
struct BaseArrayIndexTree {
    using ElementIndex = ElementIndex_t;
    using ElementIndexVector = std::vector<ElementIndex>;
    using ElementIndexIntType =
        typename detail::ArrayIndexTreeIndexType<typename ElementIndex,
                                                 typename std::is_enum<typename ElementIndex_t>::type>::IntType;
    using ElementPointerMemory = PointerMemory<ElementIndex>;

    static constexpr ElementIndex kInvalidIndex = ElementIndex(~(ElementIndex(0)));
    static_assert(std::is_integral_v<ElementIndex> || std::is_enum_v<ElementIndex>);

    static constexpr bool HasCallback = std::is_class_v<Callback_t>;
    static constexpr bool HasReleaseCallback() {
        if constexpr (HasCallback) {
            return detail::has_member_function_ReleaseElement<void (Callback_t::*)(ElementIndex, ElementIndex)>::value;
        } else {
            return false;
        }
    }
    static constexpr bool HasInitCallback() {
        if constexpr (HasCallback) {
            return detail::has_member_function_InitElement<void (Callback_t::*)(ElementIndex, ElementIndex)>::value;
        } else
            return false;
    }
    static constexpr bool HasSwapCallback() {
        if constexpr (HasCallback) {
            return detail::has_member_function_SwapValues<void (Callback_t::*)(ElementIndex, ElementIndex)>::value;
        } else {
            return false;
        }
    }
    static constexpr bool HasGetNameCallback() {
        if constexpr (HasCallback) {
            return detail::has_member_function_GetElementName<std::string (Callback_t::*)(ElementIndex) const>::value;
        } else {
            return false;
        }
    }
    static constexpr bool HasClearCallback() {
        if constexpr (HasCallback) {
            return detail::has_member_function_ClearArrays<void (Callback_t::*)()>::value;
        } else {
            return false;
        }
    }

    //---------------------------------------------------------

    template <typename T>
    using Array = T *;

    size_t element_limit = 0;
    Array<ElementIndex> parent_index = nullptr;
    Array<ElementIndex> first_child = nullptr;
    Array<ElementIndex> next_sibling = nullptr;
    Array<ElementIndex> prev_sibling = nullptr;

    //---------------------------------------------------------

    void DrawDiagram(std::ostream &out) const {
        out << "digraph {\n";
        for (ElementIndex index = 0; index < element_limit; ++index) {
            std::string name = ElementToString(index);

            if (next_sibling[index] != kInvalidIndex)
                out << "" << name << " -> " << ElementToString(next_sibling[index]) << ";\n";
            if (prev_sibling[index] != kInvalidIndex)
                out << "" << name << " -> " << ElementToString(prev_sibling[index]) << ";\n";

            if (parent_index[index] != kInvalidIndex)
                out << "" << name << " -> " << ElementToString(parent_index[index]) << " [style=dotted];\n";
            if (first_child[index] != kInvalidIndex)
                out << "" << name << " -> " << ElementToString(first_child[index]) << " [style=bold];\n";

            if (first_child[index] != kInvalidIndex) {
                out << "{ rank=same; ";
                ElementIndex f_child = first_child[index];
                if (f_child != kInvalidIndex) {
                    out << ElementToString(f_child) << "; ";
                    for (ElementIndex child = next_sibling[f_child]; child != kInvalidIndex && child != f_child;
                         child = next_sibling[child])
                        out << ElementToString(child) << "; ";
                    for (ElementIndex child = prev_sibling[f_child]; child != kInvalidIndex && child != f_child;
                         child = prev_sibling[child])
                        out << ElementToString(child) << "; ";
                }
                out << "};\n";
            }
            if (next_sibling[index] != kInvalidIndex || prev_sibling[index] != kInvalidIndex ||
                parent_index[index] != kInvalidIndex || first_child[index] != kInvalidIndex) {
                out << "{ style=filled; color=blue; " << ElementToString(index) << "; };\n";
            }
        }
        out << "}";
    }

    void DrawDiagram(const std::string &fname) const {
        std::ofstream of(fname);
        DrawDiagram(of);
    }

    void DumpArrays(std::ostream &out) const {
        auto dumpArray = [&](const char *name, Array<ElementIndex> arr) {
            out << name << " -> ";
            for (ElementIndex *it = arr, *jt = arr + element_limit; it < jt; ++it) {
                out << "\t" << (*it == kInvalidIndex ? std::string("x") : std::to_string(*it));
            }
            out << "\n";
        };
        dumpArray("parent_index", parent_index);
        dumpArray("first_child ", first_child);
        dumpArray("next_sibling", next_sibling);
        dumpArray("prev_sibling", prev_sibling);
    }

    //---------------------------------------------------------

    std::string ElementToString(ElementIndex index) const {
        if constexpr (HasGetNameCallback()) {
            return static_cast<const Callback_t *>(this)->GetElementName(index);
        } else {
            return std::to_string(index);
        }
    }

    //---------------------------------------------------------

    class DFSIterator {
        const BaseArrayIndexTree *tree;
        ElementIndex current_index;

    public:
        explicit DFSIterator(const BaseArrayIndexTree *tree, ElementIndex current_index)
            : tree(tree), current_index(current_index) {}

        DFSIterator(const DFSIterator &) = default;
        DFSIterator(DFSIterator &&) = default;

        bool operator!=(const DFSIterator &ti) const { return current_index != ti.current_index; }

        ElementIndex operator*() { return current_index; }

        void operator++() {
            ElementIndex sibling = tree->next_sibling[current_index];
            if (sibling == kInvalidIndex) {
                auto parent = tree->parent_index[current_index];
                // assert
                current_index = parent;
                return;
            }

            for (ElementIndex child = sibling; child != kInvalidIndex;) {
                auto f_child = tree->first_child[child];
                if (f_child == kInvalidIndex) {
                    current_index = child;
                    return;
                }
                child = f_child;
            }

            current_index = sibling;
        }
    };

    template <typename DFSIteratorType>
    class DFSIteratorStarter {
        BaseArrayIndexTree *tree;
        ElementIndex root_index;

    public:
        DFSIteratorStarter(BaseArrayIndexTree *tree, ElementIndex root_index) : tree(tree), root_index(root_index) {}

        DFSIteratorType begin() {
            ElementIndex current_index = root_index;
            for (;;) {
                auto child = tree->first_child[current_index];
                if (child == kInvalidIndex)
                    break;
                current_index = child;
            }
            return DFSIteratorType{tree, current_index};
        }
        DFSIteratorType end() { return DFSIteratorType{tree, tree->parent_index[root_index]}; }
    };

    //---------------------------------------------------------

    class LinearIterator {
        ElementIndex current_index;

    public:
        explicit LinearIterator(ElementIndex current_index) : current_index(current_index) {}
        LinearIterator(const LinearIterator &) = default;
        LinearIterator(LinearIterator &&) = default;

        bool operator!=(const LinearIterator &ti) const { return current_index != ti.current_index; }

        ElementIndex operator*() { return current_index; }

        void operator++() { ++current_index; }
    };

    class LinearTraverseStarter {
        ElementIndex startIndex, lastIndex;

    public:
        LinearTraverseStarter(ElementIndex startIndex, ElementIndex lastIndex)
            : startIndex(startIndex), lastIndex(lastIndex) {}
        LinearIterator begin() { return LinearIterator{startIndex}; }
        LinearIterator end() { return LinearIterator{lastIndex}; }
    };

    //---------------------------------------------------------

    DFSIteratorStarter<DFSIterator> TraverseTree(ElementIndex startIndex) {
        return DFSIteratorStarter<DFSIterator>(this, startIndex);
    }
    ElementIndexVector TraverseTreeVector(ElementIndex startIndex) {
        return Vectorize<ElementIndexVector>(TraverseTree(startIndex));
    }

    //---------------------------------------------------------

    void ReleaseBranch(ElementIndex root_index) {
        ElementIndex current_index = root_index;
        for (;;) {
            auto child = first_child[current_index];
            if (child == kInvalidIndex) {
                break;
            }
            current_index = child;
        }

        while (current_index != root_index) {
            current_index = CallReleaseIndex(current_index);
            CallReleaseElement(current_index, parent_index[current_index]);

            ElementIndex sibling = next_sibling[current_index];
            assert(prev_sibling[current_index] == kInvalidIndex);
            if (sibling == kInvalidIndex) {
                auto parent = parent_index[current_index];
                RemoveReferences(current_index);
                current_index = parent;
                continue;
            } else {
                auto parent = parent_index[current_index];
                first_child[parent] = sibling;

                prev_sibling[sibling] = kInvalidIndex;
                next_sibling[current_index] = kInvalidIndex;
                parent_index[current_index] = kInvalidIndex;
            }

            bool done = false;
            for (ElementIndex child = sibling; child != kInvalidIndex;) {
                auto f_child = first_child[child];
                if (f_child == kInvalidIndex) {
                    current_index = child;
                    done = true;
                    break;
                }
                child = f_child;
            }
            if (!done) {
                RemoveReferences(current_index);
                current_index = sibling;
            }
        }

        root_index = CallReleaseIndex(current_index);
        CallReleaseElement(root_index, parent_index[root_index]);
        RemoveReferences(root_index);
    }

    //TODO: MoveIndex(ElementIndex src, ElementIndex dst)
    void SwapIndexes(ElementIndex a, ElementIndex b) {
        assert(a != kInvalidIndex);
        assert(b != kInvalidIndex);

        CallSwapValues(a, b);

        {
            for (auto childA = first_child[a]; childA != kInvalidIndex; childA = next_sibling[childA])
                parent_index[childA] = b;

            for (auto childB = first_child[b]; childB != kInvalidIndex; childB = next_sibling[childB])
                parent_index[childB] = a;

            std::swap(first_child[a], first_child[b]);
        }

        {
            auto &a_next = next_sibling[a];
            auto &b_next = next_sibling[b];
            auto &a_prev = prev_sibling[a];
            auto &b_prev = prev_sibling[b];

            if (a_next == b) { // right next to each other
                a_next = b_next;
                b_prev = a_prev;

                if (a_next != kInvalidIndex)
                    prev_sibling[a_next] = a;

                if (b_prev != kInvalidIndex)
                    next_sibling[b_prev] = b;

                b_next = a;
                a_prev = b;
            } else if (b_next == a) { // right next to each other
                b_next = a_next;
                a_prev = b_prev;

                if (b_next != kInvalidIndex)
                    prev_sibling[b_next] = b;

                if (a_prev != kInvalidIndex)
                    next_sibling[a_prev] = a;

                a_next = b;
                b_prev = a;
            } else {
                auto p = b_prev;
                auto n = b_next;

                b_prev = a_prev;
                b_next = a_next;

                a_prev = p;
                a_next = n;

                if (b_next != kInvalidIndex)
                    prev_sibling[b_next] = b;
                if (b_prev != kInvalidIndex)
                    next_sibling[b_prev] = b;

                if (a_next != kInvalidIndex)
                    prev_sibling[a_next] = a;
                if (a_prev != kInvalidIndex)
                    next_sibling[a_prev] = a;
            }
        }

        {
            auto parentA = parent_index[a];
            auto parentB = parent_index[b];

            if (first_child[parentA] == a)
                first_child[parentA] = b;

            if (first_child[parentB] == b)
                first_child[parentB] = a;

            auto &fa = first_child[parentA];
            auto &fb = first_child[parentB];

            while (prev_sibling[fa] != kInvalidIndex)
                fa = prev_sibling[fa];
            while (prev_sibling[fb] != kInvalidIndex)
                fb = prev_sibling[fb];

            std::swap(parent_index[a], parent_index[b]);
        }
    }

protected:
    static constexpr size_t GetBaseMemorySize(ElementIndex_t element_limit) { //
        return static_cast<size_t>(element_limit) * 4;
    }

    BaseArrayIndexTree() : element_limit(0) {}
    BaseArrayIndexTree(ElementIndex_t element_limit) : element_limit(element_limit) {}
    BaseArrayIndexTree(ElementIndex_t element_limit, const ElementPointerMemory &ptr_mem)
        : element_limit(element_limit) {
        SetArrayMemory(element_limit, ptr_mem.get());
    }

    void BaseClear() {
        CallClear();
        for (ElementIndex i = 0; i < element_limit; ++i) {
            parent_index[i] = kInvalidIndex;
            first_child[i] = kInvalidIndex;
            next_sibling[i] = kInvalidIndex;
            prev_sibling[i] = kInvalidIndex;
        }
    }

    ElementIndex_t *SetArrayMemory(ElementIndex_t element_limit, ElementIndex_t *memory_block_front) {
        if (memory_block_front == nullptr) {
            return nullptr;
        }
        this->element_limit = element_limit;
        parent_index = memory_block_front;
        first_child = parent_index + element_limit;
        next_sibling = first_child + element_limit;
        prev_sibling = next_sibling + element_limit;
        return prev_sibling + element_limit;
    }

    void AddChild(ElementIndex parent, ElementIndex child) {
        assert(parent != kInvalidIndex);
        assert(child != kInvalidIndex);

        parent_index[child] = parent;
        if (first_child[parent] != kInvalidIndex) {
            auto f_child = first_child[parent];
            InsertBefore(f_child, child);
        }
        first_child[parent] = child;
    }
    void InsertBefore(ElementIndex before, ElementIndex index) {
        assert(before != kInvalidIndex);
        assert(index != kInvalidIndex);
        auto prev_before = prev_sibling[before];
        next_sibling[index] = before;
        prev_sibling[index] = prev_before;
        prev_sibling[before] = index;
        if (prev_before != kInvalidIndex)
            next_sibling[prev_before] = index;
    }
    void RemoveReferences(ElementIndex e) {
        assert(e != kInvalidIndex);
        auto prev = prev_sibling[e];
        auto next = next_sibling[e];
        auto parent = parent_index[e];

        prev_sibling[e] = kInvalidIndex;
        next_sibling[e] = kInvalidIndex;

        if (prev != kInvalidIndex)
            next_sibling[prev] = next;
        if (next != kInvalidIndex)
            prev_sibling[next] = prev;

        if (parent != kInvalidIndex && first_child[parent] == e)
            first_child[parent] = prev != kInvalidIndex ? prev : next;

        parent_index[e] = kInvalidIndex;
        first_child[e] = kInvalidIndex;
    }

    void CallInitElement(ElementIndex e, ElementIndex parent) {
        if constexpr (this->HasInitCallback()) {
            static_cast<Callback_t *>(this)->InitElement(e, parent);
        }
    }
    void CallSwapValues(ElementIndex a, ElementIndex b) {
        if constexpr (this->HasSwapCallback()) {
            static_cast<Callback_t *>(this)->SwapValues(a, b);
        }
    }
    void CallClear() {
        if constexpr (HasClearCallback()) {
            static_cast<Callback_t *>(this)->ClearArrays();
        }
    }
    void CallReleaseElement(ElementIndex e, ElementIndex parent) {
        if constexpr (this->HasReleaseCallback()) {
            static_cast<Callback_t *>(this)->ReleaseElement(e, parent);
        }
    }

    ElementIndex CallReleaseIndex(ElementIndex e) { return static_cast<Indexer_t *>(this)->ReleaseIndex(e); }

    //---------------------------------------------------------
};

} // namespace detail

template <typename ElementIndex_t, typename Callback_t = detail::ArrayIndexTreeCallback>
struct PackedArrayIndexTree
    : public detail::BaseArrayIndexTree<ElementIndex_t, PackedArrayIndexTree<ElementIndex_t, Callback_t>, Callback_t> {
    using Base = detail::BaseArrayIndexTree<ElementIndex_t, PackedArrayIndexTree, Callback_t>;
    friend class Base; //not nice

    using Base::kInvalidIndex;
    using typename Base::ElementIndex;
    using typename Base::ElementIndexIntType;
    using typename Base::ElementIndexVector;
    using typename Base::ElementPointerMemory;
    using typename Base::LinearTraverseStarter;

    //---------------------------------------------------------

    static constexpr size_t GetMemorySize(ElementIndex element_limit) { return Base::GetBaseMemorySize(element_limit); }

    PackedArrayIndexTree() = default;
    PackedArrayIndexTree(ElementIndex limit, const ElementPointerMemory &ptr_mem) : Base(limit, ptr_mem) {}

    void SetMemory(ElementIndex limit, const ElementPointerMemory &ptr_mem) {
        this->SetArrayMemory(limit, ptr_mem.get());
    }

    //---------------------------------------------------------

    ElementIndex LastIndex() const { return (ElementIndex)((ElementIndexIntType)this->allocated - 1); }

    void Clear() {
        this->BaseClear();
        allocated = 0;
    }
    ElementIndex Allocated() const { return this->allocated; }

    ElementIndex Allocate(ElementIndex parent = kInvalidIndex) {
        auto e = NextIndex();
        if (parent != kInvalidIndex) {
            this->AddChild(parent, e);
        }
        this->CallInitElement(e, parent);
        return e;
    }

    //---------------------------------------------------------

    LinearTraverseStarter LinearTraverse(ElementIndex startIndex = 0, ElementIndex lastIndex = kInvalidIndex) const {
        return LinearTraverseStarter(startIndex, lastIndex == kInvalidIndex ? allocated : lastIndex);
    }
    ElementIndexVector LinearTraverseVector(ElementIndex startIndex = 0, ElementIndex lastIndex = kInvalidIndex) const {
        return Vectorize<ElementIndexVector>(this->LinearTraverse(startIndex, lastIndex));
    }

private:
    size_t allocated = 0;

    ElementIndex ReleaseIndex(ElementIndex i) {
        ElementIndex r = LastIndex();
        if (i < r) {
            this->SwapIndexes(r, i); //TODO: move is sufficient
            SatisfyChildIndexConstraint(i);
        }
        RemoveLast();
        return r;
    }

    ElementIndex NextIndex() {
        ElementIndex ei = this->allocated;
        this->allocated = (ElementIndex)((ElementIndexIntType)this->allocated + 1);
        return ei;
    };
    void RemoveLast() { this->allocated = (ElementIndex)((ElementIndexIntType)this->allocated - 1); }

    void SatisfyChildIndexConstraint(ElementIndex e) {
        assert(e != kInvalidIndex);
        for (ElementIndex c = e; this->parent_index[c] > c;) {
            if (this->parent_index[c] == kInvalidIndex) {
                break;
            }
            this->SwapIndexes(c, this->parent_index[c]);
        }
    }
};

template <typename ElementIndex_t, typename Callback_t = detail::ArrayIndexTreeCallback>
struct SparseArrayIndexTree
    : public detail::BaseArrayIndexTree<ElementIndex_t, SparseArrayIndexTree<ElementIndex_t, Callback_t>, Callback_t> {
    using Base =
        detail::BaseArrayIndexTree<ElementIndex_t, SparseArrayIndexTree<ElementIndex_t, Callback_t>, Callback_t>;
    friend class Base; //not nice

    using Base::kInvalidIndex;
    using typename Base::ElementIndex;
    using typename Base::ElementIndexIntType;
    using typename Base::ElementIndexVector;
    using typename Base::ElementPointerMemory;
    using typename Base::LinearTraverseStarter;

    //---------------------------------------------------------

    static constexpr size_t GetMemorySize(ElementIndex element_limit) { //
        return Base::GetBaseMemorySize(element_limit);
    }

    SparseArrayIndexTree() = default;
    SparseArrayIndexTree(ElementIndex limit, const ElementPointerMemory &ptr_mem)
        : Base(limit, ptr_mem), allocation_array(limit) {}

    void SetMemory(ElementIndex limit, const ElementPointerMemory &ptr_mem) {
        this->SetArrayMemory(limit, ptr_mem.get());
        allocation_array.resize(limit);
    }

    //---------------------------------------------------------

    ElementIndex Allocate(ElementIndex parent = kInvalidIndex) {
        auto e = NextIndex(parent);
        if (parent != kInvalidIndex) {
            this->AddChild(parent, e);
        }
        this->CallInitElement(e, parent);
        return e;
    }

    ElementIndex Allocated() const { return this->element_limit - allocation_array.count(); }

    void Clear() {
        this->BaseClear();
        allocation_array.set();
    }

    //---------------------------------------------------------

    LinearTraverseStarter LinearTraverse(ElementIndex startIndex = 0, ElementIndex lastIndex = kInvalidIndex) const {
        return LinearTraverseStarter(startIndex, lastIndex == kInvalidIndex ? Allocated() : lastIndex);
    }
    ElementIndexVector LinearTraverseVector(ElementIndex startIndex = 0, ElementIndex lastIndex = kInvalidIndex) const {
        return Vectorize<ElementIndexVector>(this->LinearTraverse(startIndex, lastIndex));
    }

protected:
    boost::dynamic_bitset<> allocation_array;

    ElementIndex ReleaseIndex(ElementIndex i) {
        allocation_array.set(i, true);
        return i;
    }

    ElementIndex NextIndex(ElementIndex parent) {
        ElementIndex new_element;
        if (parent == kInvalidIndex) {
            new_element = allocation_array.find_first();
        } else {
            new_element = allocation_array.find_next(parent);
        }

        if (new_element == allocation_array.npos) {
            return kInvalidIndex;
        } else {
            allocation_array.set(new_element, false);
            return new_element;
        }
    };
};

} // namespace MoonGlare::ECS
