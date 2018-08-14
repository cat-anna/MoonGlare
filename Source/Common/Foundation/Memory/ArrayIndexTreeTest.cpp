#include <catch.hpp>
#include <iostream>
#include <fstream>

#include "ArrayIndexTree.h"

#define CATCH2_TEST_MODULE ArrayIndexTreeTest

extern bool gGenerateDiagrams;
extern bool gVerbose;

namespace MoonGlare::Memory::Test
{

TEST_CASE("AddChild", "[ArrayIndexTree]")
{
    using AIT = ArrayIndexTree<uint32_t, 16>;

    AIT ait;
    ait.Clear();

    auto first = ait.Allocate();
    REQUIRE(first == 0);
    auto second = ait.Allocate();
    REQUIRE(second == 1);
    auto third = ait.Allocate();
    REQUIRE(third == 2);

    ait.AddChild(first, second);
    ait.AddChild(first, third);

    REQUIRE(ait.firstChild[first] == third);

    REQUIRE(ait.parentIndex[second] == first);
    REQUIRE(ait.parentIndex[third] == first);

    REQUIRE(ait.nextSibling[third] == second);
    REQUIRE(ait.prevSibling[second] == third);
}

TEST_CASE("TraverseTree", "[ArrayIndexTree]")
{
    using AIT = ArrayIndexTree<uint32_t, 16>;

    AIT ait;
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

    std::vector<AIT::ElementIndex> requiredOrder = {
        e0_1_3,
        e0_1_2_0,
        e0_1_2,
        e0_1_1_0,
        e0_1_1,
        e0_1_0,
        e0_1,
        e0_0_1_0,
        e0_0_1,
        e0_0_0,
        e0_0,
        e0,
    };

    REQUIRE(ait.TraverseTreeVector(e0) == requiredOrder);
}

TEST_CASE("LinearTraverse", "[ArrayIndexTree]")
{
    using AIT = ArrayIndexTree<uint32_t, 16>;

    AIT ait;
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

    std::vector<AIT::ElementIndex> requiredOrder = {
        e0,
        e0_0,
        e0_1,
        e0_1_0,
        e0_1_1,
        e0_0_0,
        e0_0_1,
        e0_1_1_0,
        e0_0_1_0,
        e0_1_2,
        e0_1_3,
        e0_1_2_0,
    };

    REQUIRE(ait.LinearTraverseVector(e0) == requiredOrder);
}

TEST_CASE("SwapReferences", "[ArrayIndexTree]")
{
    using AIT = ArrayIndexTree<uint32_t, 16>;

    AIT ait;
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

    if (gGenerateDiagrams)
        ait.DrawDiagram("SwapReferences-before.dot");
    ait.SwapIndexes(e0_0, e0_1_1);
    if (gGenerateDiagrams)
        ait.DrawDiagram("SwapReferences-after.dot");

    std::vector<AIT::ElementIndex> requiredOrder = {
        e0_1_3,
        e0_1_2_0,
        e0_1_2,
        e0_1_1_0,
        e0_0,
        e0_1_0,
        e0_1,
        e0_0_1_0,
        e0_0_1,
        e0_0_0,
        e0_1_1,
        e0,
    };

    REQUIRE(ait.TraverseTreeVector(e0) == requiredOrder);
}

TEST_CASE("SwapSiblingReferences", "[ArrayIndexTree]")
{
    using AIT = ArrayIndexTree<uint32_t, 16>;

    AIT ait;
    ait.Clear();

    auto e0 = ait.Allocate();
    for(int i = 0; i < 10; ++i)
        ait.Allocate(e0);

    if (gGenerateDiagrams)
        ait.DrawDiagram("SwapSiblingReferences-before.dot");

    ait.SwapIndexes(6, 5);

    if (gGenerateDiagrams)
        ait.DrawDiagram("SwapSiblingReferences-after.dot");

    for (auto index : ait.LinearTraverse())
    if (ait.parentIndex[index] != ait.InvalidIndex)
        REQUIRE(ait.parentIndex[index] < index);
}

TEST_CASE("RemoveBranch", "[ArrayIndexTree]")
{
    struct AIT : public ArrayIndexTree<uint32_t, 16, AIT>
    {
        void SwapValues(ElementIndex a, ElementIndex b)
        {
            if (gVerbose)
                std::cout << "swap " << a << " <-> " << b << "\n";
        }
        void ReleaseElement(ElementIndex e, ElementIndex parent)
        {
            if (gVerbose)
                std::cout << "remove " << e << " parent(" << parent << ")\n";
        }
        void InitElemenent(ElementIndex e, ElementIndex parent)
        {
            if (gVerbose)  
                std::cout << "init " << e << " parent(" << parent << ")\n";
        }
    };

    AIT ait;
    ait.Clear();

    auto e0 = ait.Allocate();
    auto e0_0 = ait.Allocate(e0);
    auto e0_1 = ait.Allocate(e0);
    /*auto e0_1_0 =*/ ait.Allocate(e0_1);
    auto e0_1_1 = ait.Allocate(e0_1);
    /*auto e0_0_0 =*/ ait.Allocate(e0_0);
    auto e0_0_1 = ait.Allocate(e0_0);
    /*auto e0_1_1_0 =*/ ait.Allocate(e0_1_1);
    /*auto e0_0_1_0 =*/ ait.Allocate(e0_0_1);
    auto e0_1_2 = ait.Allocate(e0_1);
    /*auto e0_1_3 =*/ ait.Allocate(e0_1);
    /*auto e0_1_2_0 =*/ ait.Allocate(e0_1_2);

    if (gGenerateDiagrams)
        ait.DrawDiagram("RemoveTree-before.dot");

    ait.RemoveBranch(e0_0);

    if (gGenerateDiagrams)
        ait.DrawDiagram("RemoveTree-after.dot");

    if (gVerbose)
        ait.DumpArrays(std::cout);

    for (auto index : ait.LinearTraverse())
        if (ait.parentIndex[index] != ait.InvalidIndex)
            REQUIRE(ait.parentIndex[index] < index);

    std::vector<AIT::ElementIndex> requiredOrder = {
        6, 5, 2, 7, 4, 3, 1, 0};

    REQUIRE(ait.TraverseTreeVector(e0) == requiredOrder);
}

TEST_CASE("RemoveBranchRandom", "[ArrayIndexTree][Random]")
{
    constexpr uint32_t Limit = 4096;

    struct AIT : public ArrayIndexTree<uint32_t, Limit, AIT>
    {
        void SwapValues(ElementIndex a, ElementIndex b)
        {
            if (gVerbose)
                std::cout << "swap " << a << " <-> " << b << "\n";
        }
        void ReleaseElement(ElementIndex e, ElementIndex parent)
        {
            if (gVerbose)
                std::cout << "remove " << e << " parent(" << parent << ")\n";
        }
        void InitElemenent(ElementIndex e, ElementIndex parent)
        {
            if (gVerbose)
                std::cout << "init " << e << " parent(" << parent << ")\n";
        }
    };

    AIT ait;
    ait.Clear();

    /*auto e0 =*/ ait.Allocate();

    for (AIT::ElementIndex i = 0; i < Limit - (Limit * 0.1); ++i)
        ait.Allocate(rand() % ait.Allocated());

    if (gGenerateDiagrams)
        ait.DrawDiagram("RemoveBranchandom-start.dot");

    for (int i = 0; i < 5 && ait.Allocated() > 0; ++i)
    {
        auto r = rand() % ait.Allocated();
        if (gGenerateDiagrams)
            std::cout << "REMOVE: " << r << "\n";
        ait.RemoveBranch(r);
        if (gGenerateDiagrams)
            ait.DrawDiagram("RemoveBranchandom-pase-" + std::to_string(i) + ".dot");
        if (gVerbose)
            ait.DumpArrays(std::cout);
        for (auto index : ait.LinearTraverse())
            if (ait.parentIndex[index] != ait.InvalidIndex)
                REQUIRE(ait.parentIndex[index] < index);
    }
}

} // namespace MoonGlare::Memory::Test
