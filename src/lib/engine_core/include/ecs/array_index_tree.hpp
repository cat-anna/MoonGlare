#pragma once

#include <array>
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

} // namespace detail

template <typename ElementIndex_t, ElementIndex_t ElementLimit_v,
          typename CALLBACK = detail::ArrayIndexTreeCallback>
struct alignas(16) ArrayIndexTree {
    using ElementIndex = ElementIndex_t;
    using ElementIndexIntType = typename detail::ArrayIndexTreeIndexType<
        typename ElementIndex, typename std::is_enum<typename ElementIndex_t>::type>::IntType;

    static constexpr ElementIndex ElementLimit = ElementLimit_v;
    static constexpr ElementIndex InvalidIndex = ElementIndex(~(ElementIndex(0)));
    static_assert(std::is_integral_v<ElementIndex> || std::is_enum_v<ElementIndex>);

    using ElementIndexVector = std::vector<ElementIndex>;

    static constexpr bool HasCallback = std::is_class_v<CALLBACK>;
    static constexpr bool HasReleaseCallback() {
        if constexpr (HasCallback) {
            return detail::has_member_function_ReleaseElement<void (CALLBACK::*)(
                ElementIndex, ElementIndex)>::value;
        } else {
            return false;
        }
    }
    static constexpr bool HasInitCallback() {
        if constexpr (HasCallback) {
            return detail::has_member_function_InitElement<void (CALLBACK::*)(ElementIndex,
                                                                              ElementIndex)>::value;
        } else
            return false;
    }
    static constexpr bool HasSwapCallback() {
        if constexpr (HasCallback) {
            return detail::has_member_function_SwapValues<void (CALLBACK::*)(ElementIndex,
                                                                             ElementIndex)>::value;
        } else {
            return false;
        }
    }
    static constexpr bool HasGetNameCallback() {
        if constexpr (HasCallback) {
            // detail::has_member_function_GetElementName<std::string(CALLBACK::*) (ElementIndex)const>::value;
            return false;
        } else {
            return false;
        }
    }
    static constexpr bool HasClearCallback() {
        if constexpr (HasCallback) {
            return detail::has_member_function_ClearArrays<void (CALLBACK::*)()>::value;
        } else {
            return false;
        }
    }

    //---------------------------------------------------------

    std::string ElementToString(ElementIndex index) const {
        if constexpr (false) //HasGetNameCallback())
            return reinterpret_cast<const CALLBACK *>(this)->GetElementName(index);
        else
            return std::to_string(index);
    }

    void DrawDiagram(std::ostream &out) const {
        out << "digraph {\n";
        for (ElementIndex index = 0; index < ElementLimit; ++index) {
            std::string name = ElementToString(index);

            if (nextSibling[index] != InvalidIndex)
                out << "" << name << " -> " << ElementToString(nextSibling[index]) << ";\n";
            if (prevSibling[index] != InvalidIndex)
                out << "" << name << " -> " << ElementToString(prevSibling[index]) << ";\n";

            if (parentIndex[index] != InvalidIndex)
                out << "" << name << " -> " << ElementToString(parentIndex[index])
                    << " [style=dotted];\n";
            if (firstChild[index] != InvalidIndex)
                out << "" << name << " -> " << ElementToString(firstChild[index])
                    << " [style=bold];\n";

            if (firstChild[index] != InvalidIndex) {
                out << "{ rank=same; ";
                ElementIndex f_child = firstChild[index];
                if (f_child != InvalidIndex) {
                    out << ElementToString(f_child) << "; ";
                    for (ElementIndex child = nextSibling[f_child];
                         child != InvalidIndex && child != f_child; child = nextSibling[child])
                        out << ElementToString(child) << "; ";
                    for (ElementIndex child = prevSibling[f_child];
                         child != InvalidIndex && child != f_child; child = prevSibling[child])
                        out << ElementToString(child) << "; ";
                }
                out << "};\n";
            }
            if (index >= allocated &&
                (nextSibling[index] != InvalidIndex || prevSibling[index] != InvalidIndex ||
                 parentIndex[index] != InvalidIndex || firstChild[index] != InvalidIndex)) {
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
        auto dumpArray = [&](const char *name, const Array<ElementIndex> &arr) {
            out << name << " -> ";
            for (auto it : arr) {
                out << "\t" << (it == InvalidIndex ? std::string("x") : std::to_string(it));
            }
            out << "\n";
        };
        dumpArray("parentIndex", parentIndex);
        dumpArray("firstChild ", firstChild);
        dumpArray("nextSibling", nextSibling);
        dumpArray("prevSibling", prevSibling);
    }

    //---------------------------------------------------------

    void Clear() {
        if constexpr (HasClearCallback())
            reinterpret_cast<CALLBACK *>(this)->ClearArrays();
        allocated = (ElementIndex)0;
        parentIndex.fill(InvalidIndex);
        firstChild.fill(InvalidIndex);
        nextSibling.fill(InvalidIndex);
        prevSibling.fill(InvalidIndex);
    }

    ElementIndex Allocated() { return allocated; }
    ElementIndex LastIndex() const { return (ElementIndex)((ElementIndexIntType)allocated - 1); }

    ElementIndex Allocate(ElementIndex parent = InvalidIndex) {
        auto e = NextIndex();
        if (parent != InvalidIndex)
            AddChild(parent, e);
        if constexpr (HasInitCallback())
            reinterpret_cast<CALLBACK *>(this)->InitElement(e, parent);
        return e;
    }

    void RemoveBranch(ElementIndex rootIndex) {
        auto remove = [this](ElementIndex i) -> ElementIndex {
            ElementIndex r = i;

            // static int idx = 0;
            // DrawDiagram("remove_" + std::to_string(idx) + ".dot");
            // std::cout << "STORED REMOVE " << idx << "\n";
            // ++idx;

            r = LastIndex();
            if (i < r) {
                SwapIndexes(r, i); //move is sufficient
                SatisfyChildIndexConstraint(i);
            }
            // RemoveReferences(r);

            if constexpr (HasReleaseCallback())
                reinterpret_cast<CALLBACK *>(this)->ReleaseElement(r, parentIndex[r]);

            RemoveLast();

            return r;
        };

        ElementIndex currentIndex = rootIndex;
        for (;;) {
            auto child = firstChild[currentIndex];
            if (child == InvalidIndex)
                break;
            currentIndex = child;
        }

        while (currentIndex != rootIndex) {
            currentIndex = remove(currentIndex);

            ElementIndex sibling = nextSibling[currentIndex];
            assert(prevSibling[currentIndex] == InvalidIndex);
            if (sibling == InvalidIndex) {
                auto parent = parentIndex[currentIndex];
                RemoveReferences(currentIndex);
                currentIndex = parent;
                continue;
            } else {
                auto parent = parentIndex[currentIndex];
                firstChild[parent] = sibling;

                prevSibling[sibling] = InvalidIndex;
                nextSibling[currentIndex] = InvalidIndex;
                parentIndex[currentIndex] = InvalidIndex;
            }

            bool done = false;
            for (ElementIndex child = sibling; child != InvalidIndex;) {
                auto f_child = firstChild[child];
                if (f_child == InvalidIndex) {
                    currentIndex = child;
                    done = true;
                    break;
                }
                child = f_child;
            }
            if (!done) {
                RemoveReferences(currentIndex);
                currentIndex = sibling;
            }
        }

        rootIndex = remove(currentIndex);
        RemoveReferences(rootIndex);
    }

    // TODO void SetParent(ElementIndex e, ElementIndex newParent) { }
    // TODO void RebuildTree(ElementIndex startIndex) { ... }

    //---------------------------------------------------------

    class DFSIterator {
        const ArrayIndexTree *tree;
        ElementIndex currentIndex;

    public:
        explicit DFSIterator(const ArrayIndexTree *tree, ElementIndex currentIndex)
            : tree(tree), currentIndex(currentIndex) {}

        DFSIterator(const DFSIterator &) = default;
        DFSIterator(DFSIterator &&) = default;

        bool operator!=(const DFSIterator &ti) const { return currentIndex != ti.currentIndex; }

        ElementIndex operator*() { return currentIndex; }

        void operator++() {
            ElementIndex sibling = tree->nextSibling[currentIndex];
            if (sibling == InvalidIndex) {
                auto parent = tree->parentIndex[currentIndex];
                // assert
                currentIndex = parent;
                return;
            }

            for (ElementIndex child = sibling; child != InvalidIndex;) {
                auto f_child = tree->firstChild[child];
                if (f_child == InvalidIndex) {
                    currentIndex = child;
                    return;
                }
                child = f_child;
            }

            currentIndex = sibling;
        }
    };

    template <typename DFSIteratorType>
    class DFSIteratorStarter {
        ArrayIndexTree *tree;
        ElementIndex rootIndex;

    public:
        DFSIteratorStarter(ArrayIndexTree *tree, ElementIndex rootIndex)
            : tree(tree), rootIndex(rootIndex) {}

        DFSIteratorType begin() {
            ElementIndex currentIndex = rootIndex;
            for (;;) {
                auto child = tree->firstChild[currentIndex];
                if (child == InvalidIndex)
                    break;
                currentIndex = child;
            }
            return DFSIteratorType{tree, currentIndex};
        }
        DFSIteratorType end() { return DFSIteratorType{tree, tree->parentIndex[rootIndex]}; }
    };

    DFSIteratorStarter<DFSIterator> TraverseTree(ElementIndex startIndex) {
        return DFSIteratorStarter<DFSIterator>(this, startIndex);
    }
    ElementIndexVector TraverseTreeVector(ElementIndex startIndex) {
        return Vectorize(TraverseTree(startIndex));
    }

    //---------------------------------------------------------

    class LinearIterator {
        ElementIndex currentIndex;

    public:
        explicit LinearIterator(ElementIndex currentIndex) : currentIndex(currentIndex) {}
        LinearIterator(const LinearIterator &) = default;
        LinearIterator(LinearIterator &&) = default;

        bool operator!=(const LinearIterator &ti) const { return currentIndex != ti.currentIndex; }

        ElementIndex operator*() { return currentIndex; }

        void operator++() { ++currentIndex; }
    };

    class LinearTraverseStarter {
        ElementIndex startIndex, lastIndex;

    public:
        LinearTraverseStarter(ElementIndex startIndex, ElementIndex lastIndex)
            : startIndex(startIndex), lastIndex(lastIndex) {}
        LinearIterator begin() { return LinearIterator{startIndex}; }
        LinearIterator end() { return LinearIterator{lastIndex}; }
    };

    LinearTraverseStarter LinearTraverse(ElementIndex startIndex = 0,
                                         ElementIndex lastIndex = InvalidIndex) const {
        return LinearTraverseStarter(startIndex, lastIndex == InvalidIndex ? allocated : lastIndex);
    }
    ElementIndexVector LinearTraverseVector(ElementIndex startIndex = 0,
                                            ElementIndex lastIndex = InvalidIndex) const {
        return Vectorize(LinearTraverse(startIndex, lastIndex));
    }

    //---------------------------------------------------------

    //TODO: TraverseChildren

    //---------------------------------------------------------

    template <typename T>
    using Array = typename std::array<T, ElementLimit>;

    //TODO: MoveIndex(ElementIndex src, ElementIndex dst)
    void SwapIndexes(ElementIndex a, ElementIndex b) {
        assert(a != InvalidIndex);
        assert(b != InvalidIndex);

        if constexpr (HasSwapCallback())
            reinterpret_cast<CALLBACK *>(this)->SwapValues(a, b);

        {
            for (auto childA = firstChild[a]; childA != InvalidIndex; childA = nextSibling[childA])
                parentIndex[childA] = b;

            for (auto childB = firstChild[b]; childB != InvalidIndex; childB = nextSibling[childB])
                parentIndex[childB] = a;

            std::swap(firstChild[a], firstChild[b]);
        }

        {
            auto &a_next = nextSibling[a];
            auto &b_next = nextSibling[b];
            auto &a_prev = prevSibling[a];
            auto &b_prev = prevSibling[b];

            if (a_next == b) { // right next to each other
                a_next = b_next;
                b_prev = a_prev;

                if (a_next != InvalidIndex)
                    prevSibling[a_next] = a;

                if (b_prev != InvalidIndex)
                    nextSibling[b_prev] = b;

                b_next = a;
                a_prev = b;
            } else if (b_next == a) { // right next to each other
                b_next = a_next;
                a_prev = b_prev;

                if (b_next != InvalidIndex)
                    prevSibling[b_next] = b;

                if (a_prev != InvalidIndex)
                    nextSibling[a_prev] = a;

                a_next = b;
                b_prev = a;
            } else {
                auto p = b_prev;
                auto n = b_next;

                b_prev = a_prev;
                b_next = a_next;

                a_prev = p;
                a_next = n;

                if (b_next != InvalidIndex)
                    prevSibling[b_next] = b;
                if (b_prev != InvalidIndex)
                    nextSibling[b_prev] = b;

                if (a_next != InvalidIndex)
                    prevSibling[a_next] = a;
                if (a_prev != InvalidIndex)
                    nextSibling[a_prev] = a;
            }
        }

        {
            auto parentA = parentIndex[a];
            auto parentB = parentIndex[b];

            if (firstChild[parentA] == a)
                firstChild[parentA] = b;

            if (firstChild[parentB] == b)
                firstChild[parentB] = a;

            auto &fa = firstChild[parentA];
            auto &fb = firstChild[parentB];

            while (prevSibling[fa] != InvalidIndex)
                fa = prevSibling[fa];
            while (prevSibling[fb] != InvalidIndex)
                fb = prevSibling[fb];

            std::swap(parentIndex[a], parentIndex[b]);
        }

        // static int idx = 0;
        // DrawDiagram("swap_" + std::to_string(idx) + ".dot");
        // std::cout << "STORED SWAP " << idx << "\n";
        // ++idx;
    }

private:
    ElementIndex allocated; // = 0;
    char __padding[16 - sizeof(ElementIndex)];

public:
    Array<ElementIndex> parentIndex;
    Array<ElementIndex> firstChild;
    Array<ElementIndex> nextSibling;
    Array<ElementIndex> prevSibling;

private:
    ElementIndex NextIndex() {
        ElementIndex ei = allocated;
        allocated = (ElementIndex)((ElementIndexIntType)allocated + 1);
        return ei;
    };
    void RemoveLast() { allocated = (ElementIndex)((ElementIndexIntType)allocated - 1); }

    void AddChild(ElementIndex parent, ElementIndex child) {
        assert(parent != InvalidIndex);
        assert(child != InvalidIndex);

        parentIndex[child] = parent;
        if (firstChild[parent] != InvalidIndex) {
            auto f_child = firstChild[parent];
            InsertBefore(f_child, child);
        }
        firstChild[parent] = child;
    }
    void InsertBefore(ElementIndex before, ElementIndex index) {
        assert(before != InvalidIndex);
        assert(index != InvalidIndex);
        auto prevBefore = prevSibling[before];
        nextSibling[index] = before;
        prevSibling[index] = prevBefore;
        prevSibling[before] = index;
        if (prevBefore != InvalidIndex)
            nextSibling[prevBefore] = index;
    }
    void RemoveReferences(ElementIndex e) {
        assert(e != InvalidIndex);
        auto prev = prevSibling[e];
        auto next = nextSibling[e];
        auto parent = parentIndex[e];

        prevSibling[e] = InvalidIndex;
        nextSibling[e] = InvalidIndex;

        if (prev != InvalidIndex)
            nextSibling[prev] = next;
        if (next != InvalidIndex)
            prevSibling[next] = prev;

        if (parent != InvalidIndex && firstChild[parent] == e)
            firstChild[parent] = prev != InvalidIndex ? prev : next;

        parentIndex[e] = InvalidIndex;
        firstChild[e] = InvalidIndex;
    }
    void SatisfyChildIndexConstraint(ElementIndex e) {
        assert(e != InvalidIndex);
        for (ElementIndex c = e; parentIndex[c] > c;) {
            if (parentIndex[c] == InvalidIndex)
                break;
            SwapIndexes(c, parentIndex[c]);
        }
    }

    template <typename STARTER>
    ElementIndexVector Vectorize(STARTER st) const {
        ElementIndexVector r;
        for (ElementIndex idx : st)
            r.emplace_back(idx);
        return r;
    }
};

namespace detail {
struct TestArrayIndexTree : public ArrayIndexTree<uint32_t, 16, TestArrayIndexTree> {};
static_assert(std::is_trivial_v<TestArrayIndexTree>);
static_assert(std::is_trivially_constructible_v<TestArrayIndexTree>);
static_assert((sizeof(TestArrayIndexTree) % 16) == 0);
} // namespace detail

} // namespace MoonGlare::ECS