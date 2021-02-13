#include "ecs/array_index_tree.hpp"
#include <catch.hpp>
#include <fstream>
#include <iostream>

constexpr bool kGenerateDiagrams = false;
constexpr bool kVerbose = false;

namespace MoonGlare::ECS {

struct AITCallbacks {
    using Index = uint32_t; //not nice

    void SwapValues(Index a, Index b) {
        if (kVerbose) {
            std::cout << "swap " << a << " <-> " << b << "\n";
        }
    }
    void ReleaseElement(Index e, Index parent) {
        if (kVerbose) {
            std::cout << "remove " << e << " parent(" << parent << ")\n";
        }
    }
    void InitElement(Index e, Index parent) {
        if (kVerbose) {
            std::cout << "init " << e << " parent(" << parent << ")\n";
        }
    }
};

struct PackedAIT : public PackedArrayIndexTree<uint32_t, PackedAIT>, public AITCallbacks {
    using Base = PackedArrayIndexTree<uint32_t, AITCallbacks>;
    template <typename... ARGS>
    PackedAIT(ARGS &&... args) : PackedArrayIndexTree(std::forward<ARGS>(args)...) {}
};

struct SparseAIT : public SparseArrayIndexTree<uint32_t, PackedAIT>, public AITCallbacks {
    using Base = SparseArrayIndexTree<uint32_t, AITCallbacks>;
    template <typename... ARGS>
    SparseAIT(ARGS &&... args) : SparseArrayIndexTree(std::forward<ARGS>(args)...) {}
};

TEMPLATE_TEST_CASE("AddChild", "[ArrayIndexTree]", PackedAIT, SparseAIT) {
    constexpr uint32_t kLimit = 16;
    PointerMemory<TestType::ElementIndex> memory(TestType::GetMemorySize(kLimit));
    TestType ait(kLimit, memory);
    ait.Clear();

    auto first = ait.Allocate();
    REQUIRE(first == 0);
    auto second = ait.Allocate(first);
    REQUIRE(second == 1);
    auto third = ait.Allocate(first);
    REQUIRE(third == 2);

    REQUIRE(ait.Allocated() == 3);

    REQUIRE(ait.first_child[first] == third);

    REQUIRE(ait.parent_index[second] == first);
    REQUIRE(ait.parent_index[third] == first);

    REQUIRE(ait.next_sibling[third] == second);
    REQUIRE(ait.prev_sibling[second] == third);
}

TEMPLATE_TEST_CASE("TraverseTree", "[ArrayIndexTree]", PackedAIT) {
    constexpr uint32_t kLimit = 16;
    PointerMemory<TestType::ElementIndex> memory(TestType::GetMemorySize(kLimit));
    TestType ait(kLimit, memory);

    ait.Clear();

    auto e0 = ait.Allocate();
    auto e0_0 = ait.Allocate(e0);
    auto e0_1 = ait.Allocate(e0);
    REQUIRE(ait.Allocated() == 3);

    auto e0_1_0 = ait.Allocate(e0_1);
    auto e0_1_1 = ait.Allocate(e0_1);

    auto e0_0_0 = ait.Allocate(e0_0);
    auto e0_0_1 = ait.Allocate(e0_0);

    auto e0_1_1_0 = ait.Allocate(e0_1_1);

    auto e0_0_1_0 = ait.Allocate(e0_0_1);

    auto e0_1_2 = ait.Allocate(e0_1);
    auto e0_1_3 = ait.Allocate(e0_1);

    auto e0_1_2_0 = ait.Allocate(e0_1_2);

    typename TestType::ElementIndexVector requiredOrder = {
        e0_1_3, e0_1_2_0, e0_1_2, e0_1_1_0, e0_1_1, e0_1_0, e0_1, e0_0_1_0, e0_0_1, e0_0_0, e0_0, e0,
    };

    REQUIRE(ait.TraverseTreeVector(e0) == requiredOrder);
}

TEMPLATE_TEST_CASE("LinearTraverse", "[ArrayIndexTree]", PackedAIT, SparseAIT) {
    constexpr uint32_t kLimit = 16;
    PointerMemory<TestType::ElementIndex> memory(TestType::GetMemorySize(kLimit));
    TestType ait(kLimit, memory);
    ait.Clear();

    auto e0 = ait.Allocate();
    auto e0_0 = ait.Allocate(e0);
    auto e0_1 = ait.Allocate(e0);

    auto e0_1_0 = ait.Allocate(e0_1);
    auto e0_1_1 = ait.Allocate(e0_1);

    auto e0_0_0 = ait.Allocate(e0_0);
    auto e0_0_1 = ait.Allocate(e0_0);

    auto e0_1_1_0 = ait.Allocate(e0_1_1);

    auto e0_0_1_0 = ait.Allocate(e0_0_1);

    auto e0_1_2 = ait.Allocate(e0_1);
    auto e0_1_3 = ait.Allocate(e0_1);

    auto e0_1_2_0 = ait.Allocate(e0_1_2);

    typename TestType::ElementIndexVector requiredOrder = {
        e0, e0_0, e0_1, e0_1_0, e0_1_1, e0_0_0, e0_0_1, e0_1_1_0, e0_0_1_0, e0_1_2, e0_1_3, e0_1_2_0,
    };

    REQUIRE(ait.LinearTraverseVector(e0) == requiredOrder);
}

TEMPLATE_TEST_CASE("SwapReferences", "[ArrayIndexTree]", PackedAIT, SparseAIT) {
    constexpr uint32_t kLimit = 16;
    PointerMemory<TestType::ElementIndex> memory(TestType::GetMemorySize(kLimit));
    TestType ait(kLimit, memory);
    ait.Clear();

    auto e0 = ait.Allocate();
    auto e0_0 = ait.Allocate(e0);
    auto e0_1 = ait.Allocate(e0);
    auto e0_1_0 = ait.Allocate(e0_1);
    auto e0_1_1 = ait.Allocate(e0_1);
    auto e0_0_0 = ait.Allocate(e0_0);
    auto e0_0_1 = ait.Allocate(e0_0);
    auto e0_1_1_0 = ait.Allocate(e0_1_1);
    auto e0_0_1_0 = ait.Allocate(e0_0_1);
    auto e0_1_2 = ait.Allocate(e0_1);
    auto e0_1_3 = ait.Allocate(e0_1);
    auto e0_1_2_0 = ait.Allocate(e0_1_2);

    if (kGenerateDiagrams) {
        ait.DrawDiagram("SwapReferences-before.dot");
    }
    ait.SwapIndexes(e0_0, e0_1_1);
    if (kGenerateDiagrams) {
        ait.DrawDiagram("SwapReferences-after.dot");
    }

    typename TestType::ElementIndexVector requiredOrder = {
        e0_1_3, e0_1_2_0, e0_1_2, e0_1_1_0, e0_0, e0_1_0, e0_1, e0_0_1_0, e0_0_1, e0_0_0, e0_1_1, e0,
    };

    REQUIRE(ait.TraverseTreeVector(e0) == requiredOrder);
}

TEMPLATE_TEST_CASE("SwapSiblingReferences", "[ArrayIndexTree]", PackedAIT, SparseAIT) {
    constexpr uint32_t kLimit = 16;
    PointerMemory<TestType::ElementIndex> memory(TestType::GetMemorySize(kLimit));
    TestType ait(kLimit, memory);
    ait.Clear();

    auto e0 = ait.Allocate();
    for (int i = 0; i < 10; ++i) {
        ait.Allocate(e0);
    }

    if (kGenerateDiagrams) {
        ait.DrawDiagram("SwapSiblingReferences-before.dot");
    }

    ait.SwapIndexes(6, 5);

    if (kGenerateDiagrams) {
        ait.DrawDiagram("SwapSiblingReferences-after.dot");
    }

    for (auto index : ait.LinearTraverse()) {
        if (ait.parent_index[index] != ait.kInvalidIndex) {
            REQUIRE(ait.parent_index[index] < index);
        }
    }
}

TEMPLATE_TEST_CASE("ReleaseBranch", "[ArrayIndexTree]", PackedAIT, SparseAIT) {
    constexpr uint32_t kLimit = 16;
    PointerMemory<TestType::ElementIndex> memory(TestType::GetMemorySize(kLimit));
    TestType ait(kLimit, memory);
    ait.Clear();

    auto e0 = ait.Allocate();
    auto e0_0 = ait.Allocate(e0);
    auto e0_1 = ait.Allocate(e0);
    /*auto e0_1_0 =*/ait.Allocate(e0_1);
    auto e0_1_1 = ait.Allocate(e0_1);
    /*auto e0_0_0 =*/ait.Allocate(e0_0);
    auto e0_0_1 = ait.Allocate(e0_0);
    /*auto e0_1_1_0 =*/ait.Allocate(e0_1_1);
    /*auto e0_0_1_0 =*/ait.Allocate(e0_0_1);
    auto e0_1_2 = ait.Allocate(e0_1);
    /*auto e0_1_3 =*/ait.Allocate(e0_1);
    /*auto e0_1_2_0 =*/ait.Allocate(e0_1_2);

    if (kGenerateDiagrams) {
        ait.DrawDiagram("RemoveTree-before.dot");
    }

    ait.ReleaseBranch(e0_0);

    if (kGenerateDiagrams) {
        ait.DrawDiagram("RemoveTree-after.dot");
    }

    if (kVerbose) {
        ait.DumpArrays(std::cout);
    }

    for (auto index : ait.LinearTraverse()) {
        if (ait.parent_index[index] != ait.kInvalidIndex) {
            REQUIRE(ait.parent_index[index] < index);
        }
    }

    typename TestType::ElementIndexVector requiredOrder;

    // valid indexes are different in sparse and packed
    if constexpr (std::is_same_v<TestType, PackedAIT>) {
        requiredOrder = {6, 5, 2, 7, 4, 3, 1, 0};
    }
    if constexpr (std::is_same_v<TestType, SparseAIT>) {
        requiredOrder = {10, 11, 9, 7, 4, 3, 2, 0};
    }

    auto valid_indexes = ait.TraverseTreeVector(e0);
    REQUIRE(valid_indexes.size() == 8);
    REQUIRE(valid_indexes == requiredOrder);
}

TEMPLATE_TEST_CASE("ReleaseBranchRandom", "[PackedArrayIndexTree][Random]", PackedAIT, SparseAIT) {
    constexpr uint32_t kLimit = 4096;

    PointerMemory<TestType::ElementIndex> memory(TestType::GetMemorySize(kLimit));
    TestType ait(kLimit, memory);
    ait.Clear();

    /*auto e0 =*/ait.Allocate();

    for (typename TestType::ElementIndex i = 0; i < kLimit - (kLimit * 0.1); ++i) {
        ait.Allocate(rand() % ait.Allocated());
    }

    if (kGenerateDiagrams) {
        ait.DrawDiagram("ReleaseBranchandom-start.dot");
    }

    for (int i = 0; i < 5 && ait.Allocated() > 0; ++i) {
        auto r = rand() % ait.Allocated();
        if (kGenerateDiagrams) {
            std::cout << "REMOVE: " << r << "\n";
        }
        ait.ReleaseBranch(r);
        if (kGenerateDiagrams) {
            ait.DrawDiagram("ReleaseBranchandom-pase-" + std::to_string(i) + ".dot");
        }
        if (kVerbose) {
            ait.DumpArrays(std::cout);
        }
        for (auto index : ait.LinearTraverse()) {
            if (ait.parent_index[index] != ait.kInvalidIndex) {
                REQUIRE(ait.parent_index[index] < index);
            }
        }
    }
}

} // namespace MoonGlare::ECS
