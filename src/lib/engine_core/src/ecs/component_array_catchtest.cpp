#include "ecs/component_array.hpp"
#include "ecs/ecs_configuration.hpp"
#include "ecs/ecs_register.hpp"
#include "static_string.hpp"
#include <array>
#include <catch.hpp>
#include <set>

namespace MoonGlare::ECS {

constexpr uint64_t kConstructorTag = 3012559397453824;
constexpr uint64_t kDestructorTag = 2569567136120832;

#pragma pack(push, 1)
struct CommonTags {
    uint64_t tag = kConstructorTag;
    ~CommonTags() { tag = kDestructorTag; }

    bool IsConstructed() const { return tag == kConstructorTag; }
    bool IsDestructed() const { return tag == kDestructorTag; }
};

struct ComponentA : public CommonTags {
    static constexpr ComponentId kComponentId = 0;
    static constexpr char kComponentName[] = "a";
    int v = 1;
    ComponentA(){};
    ComponentA(int _v) : v(_v){};
};

struct alignas(16) ComponentB : public CommonTags {
    static constexpr ComponentId kComponentId = 1;
    static constexpr char kComponentName[] = "b";

    uint64_t v = 2;
    char c; //make size non %16

    ComponentB(){};
    ComponentB(uint64_t _v) : v(_v){};
};

struct ComponentC : public CommonTags {
    static constexpr ComponentId kComponentId = 0x10;
    static constexpr char kComponentName[] = "c";

    using StringType = BasicStaticString<32, char>;
    StringType v;
};

#pragma pack(pop)
struct TestComponentRegister : public ECSRegister {
    TestComponentRegister() {
        RegisterComponent<ComponentA>();
        RegisterComponent<ComponentB>();
        RegisterComponent<ComponentC>();
    }
};

TEST_CASE("MakeComponentMask", "[ComponentArray]") {
    using namespace ECS::detail;

    REQUIRE(MakeComponentMask<ComponentA>() == 0x1llu);
    REQUIRE(MakeComponentMask(ComponentA::kComponentId) == 0x1llu);
    REQUIRE(MakeComponentMask<ComponentA, ComponentB>() == 0x3llu);
    REQUIRE(MakeComponentMask<ComponentB, ComponentC>() == 0x10002llu);

    REQUIRE(MakeComponentActiveMask<ComponentA>() == 0x4000'0000);
    REQUIRE(MakeComponentActiveMask(ComponentA::kComponentId) == 0x4000'0000llu);
    REQUIRE(MakeComponentActiveMask<ComponentA, ComponentB>() == 0xc000'0000llu);
    REQUIRE(MakeComponentActiveMask<ComponentB, ComponentC>() == 0x4000'8000'0000llu);

    REQUIRE(MakeComponentMaskWithActiveFlag<ComponentA>() == 0x4000'0001llu);
    REQUIRE(MakeComponentMaskWithActiveFlag(ComponentA::kComponentId) == 0x4000'0001llu);
    REQUIRE(MakeComponentMaskWithActiveFlag<ComponentA, ComponentB>() == 0xc000'0003llu);
    REQUIRE(MakeComponentMaskWithActiveFlag<ComponentB, ComponentC>() == 0x4000'8001'0002llu);

    REQUIRE(MakeComponentMask<ComponentA, ComponentB>(Component::ComponentFlags::kValid) == 0x1000'0000'0000'0003llu);
}

TEST_CASE("Unique addresses", "[ComponentArray]") {
    TestComponentRegister c_register;
    ComponentArray c_array(&c_register);

    auto &page = c_array.GetComponentPage();

    REQUIRE(page.valid_components_and_flags != nullptr);

    std::set<void *> array_addresses;
    array_addresses.insert(page.valid_components_and_flags);
    for (auto item : page.component_array) {
        array_addresses.insert(item.array_front);
    }

    REQUIRE(array_addresses.size() == page.component_array.size() + 1);
}

TEST_CASE("Add component", "[ComponentArray]") {
    TestComponentRegister c_register;
    ComponentArray c_array(&c_register);

    auto index = 8llu;

    SECTION("Index is not valid") {
        REQUIRE(c_array.AssignComponent<ComponentA>(index, 5) == nullptr);
        REQUIRE(c_array.CreateComponent(index, ComponentA::kComponentId) == nullptr);
    }

    c_array.MarkIndexAsValid(index);

    SECTION("add by type") {
        auto *a = c_array.AssignComponent<ComponentA>(index, 5);
        REQUIRE(c_array.HasComponent<ComponentA>(index));
        REQUIRE(a != nullptr);
        REQUIRE(a->v == 5);
        REQUIRE(a->IsConstructed());
        REQUIRE(a == c_array.AssignComponent<ComponentA>(index, 6));
        REQUIRE(a->v == 6);
        REQUIRE(a->IsConstructed());

        c_array.MarkIndexAsValid(index - 1);
        auto *a1 = c_array.AssignComponent<ComponentA>(index - 1, 1);
        REQUIRE(c_array.HasComponent<ComponentA>(index - 1));
        REQUIRE(a1 != nullptr);
        REQUIRE(a1->IsConstructed());
        REQUIRE(a1->v == 1);
        REQUIRE(a->v == 6);
        REQUIRE(a1 < a);
    }

    SECTION("add by id") {
        c_array.MarkIndexAsValid(index + 1);
        auto *a = (uint8_t *)c_array.CreateComponent(index, ComponentA::kComponentId);
        auto *b = (uint8_t *)c_array.CreateComponent(index, ComponentB::kComponentId);
        auto *a1 = (uint8_t *)c_array.CreateComponent(index + 1, ComponentA::kComponentId);
        REQUIRE(c_array.HasComponent(index, ComponentA::kComponentId));
        REQUIRE(a != nullptr);
        REQUIRE(b != nullptr);
        REQUIRE(a1 != nullptr);
        REQUIRE(a1 > a);
        REQUIRE(a + sizeof(ComponentA) == a1);
        REQUIRE(reinterpret_cast<ComponentB *>(b)->v == 2);
    }

    SECTION("check alignment") {
        c_array.MarkIndexAsValid(index + 1);
        auto *a0 = (uint8_t *)c_array.CreateComponent(index, ComponentA::kComponentId);
        auto *a1 = (uint8_t *)c_array.CreateComponent(index + 1, ComponentA::kComponentId);

        auto *b0 = (uint8_t *)c_array.CreateComponent(index, ComponentB::kComponentId);
        auto *b1 = (uint8_t *)c_array.CreateComponent(index + 1, ComponentB::kComponentId);

        REQUIRE(a1 - a0 == 12);
        REQUIRE(b1 - b0 == 32);
    }

    SECTION("add different types") {
        auto *a = c_array.AssignComponent<ComponentA>(index, 5);
        auto *b = c_array.AssignComponent<ComponentB>(index, 1);
        REQUIRE(c_array.HasComponent<ComponentA>(index));
        REQUIRE(c_array.HasComponent<ComponentB>(index));
        REQUIRE(!c_array.HasComponent<ComponentC>(index));
        REQUIRE(a != nullptr);
        REQUIRE(b != nullptr);
        REQUIRE(a->IsConstructed());
        REQUIRE(b->IsConstructed());
        REQUIRE((void *)a != (void *)b);
        REQUIRE(a->v == 5);
        REQUIRE(b->v == 1);
    }

    SECTION("destructor is called") {
        auto *b_ptr = (uint8_t *)c_array.CreateComponent(index, ComponentB::kComponentId);
        auto b = reinterpret_cast<ComponentB *>(b_ptr);
        REQUIRE(b->IsConstructed());
        c_array.RemoveComponent<ComponentB>(index);
        REQUIRE(!c_array.HasComponent<ComponentB>(index));
        REQUIRE(b->IsDestructed());
    }

    SECTION("get returns same pointer") {
        auto *b_ptr = (uint8_t *)c_array.CreateComponent(index, ComponentB::kComponentId);
        auto b = reinterpret_cast<ComponentB *>(b_ptr);
        REQUIRE(b_ptr == c_array.GetComponent(index, ComponentB::kComponentId));
    }

    SECTION("active state") {
        auto *a = c_array.AssignComponent<ComponentA>(index, 5);
        auto *b = c_array.AssignComponent<ComponentB>(index, 1);
        REQUIRE(c_array.IsComponentActive<ComponentA>(index));
        REQUIRE(c_array.IsComponentActive<ComponentB>(index));
        c_array.SetComponentActive<ComponentA>(index, false);
        REQUIRE(!c_array.IsComponentActive<ComponentA>(index));
        c_array.SetComponentActive<ComponentA>(index, true);
        REQUIRE(c_array.IsComponentActive<ComponentA>(index));
    }

    SECTION("destroy all") {
        auto *a = c_array.AssignComponent<ComponentA>(index, 5);
        auto *b = c_array.AssignComponent<ComponentB>(index, 1);
        auto *c = c_array.AssignComponent<ComponentC>(index);
        REQUIRE(a->IsConstructed());
        REQUIRE(b->IsConstructed());
        REQUIRE(c->IsConstructed());
        c_array.RemoveAllComponents(index);
        REQUIRE(!c_array.HasComponent<ComponentA>(index));
        REQUIRE(!c_array.HasComponent<ComponentB>(index));
        REQUIRE(!c_array.HasComponent<ComponentC>(index));
        REQUIRE(a->IsDestructed());
        REQUIRE(b->IsDestructed());
        REQUIRE(c->IsDestructed());
    }
}

template <uint64_t c>
struct RepeatCount {
    constexpr static uint64_t Count = c;
};

//RepeatCount<kComponentPageSize>
TEMPLATE_TEST_CASE("visit", "[ComponentArray]", RepeatCount<64>, RepeatCount<4096>) {
    TestComponentRegister c_register;
    ComponentArray c_array(&c_register);

    constexpr size_t kTestElementCount = TestType::Count;

    for (int i = 0; i < kTestElementCount; ++i) {
        c_array.MarkIndexAsValid(i);
        c_array.AssignComponent<ComponentA>(i, i);
        c_array.AssignComponent<ComponentB>(i, i);
        if ((i & 1) == 0) {
            c_array.SetComponentActive<ComponentB>(i, false);
        }
        if (i & 1) {
            auto *c = c_array.AssignComponent<ComponentC>(i);
            REQUIRE(c != nullptr);
            c->v = fmt::format("C {}", i);
        }
    }

    SECTION("visit single") {
        std::set<void *> visited;
        c_array.Visit<ComponentA>([&visited](ComponentA &a) {
            REQUIRE(&a != nullptr);
            REQUIRE(a.v >= 0);
            REQUIRE(a.v < kTestElementCount);
            visited.insert(&a);
        });
        REQUIRE(visited.size() == kTestElementCount);
    }

    SECTION("visit only active") {
        std::set<void *> visited;
        c_array.Visit<ComponentA, ComponentB>([&visited](ComponentA &a, ComponentB &b) {
            REQUIRE(&a != nullptr);
            REQUIRE(&b != nullptr);
            REQUIRE((a.v & 1) == 1);
            REQUIRE(a.v >= 0);
            REQUIRE(a.v < kTestElementCount);
            REQUIRE((b.v & 1) == 1);
            REQUIRE(b.v >= 0);
            REQUIRE(b.v < kTestElementCount);
            visited.insert(&a);
        });
        REQUIRE(visited.size() == kTestElementCount / 2);
    }

    SECTION("visit only created") {
        std::set<void *> visited;
        c_array.Visit<ComponentC>([&visited](ComponentC &c) {
            REQUIRE(&c != nullptr);
            REQUIRE(c.v.size() >= 3);
            visited.insert(&c);
        });
        REQUIRE(visited.size() == kTestElementCount / 2);
    }

    SECTION("dont visit removed") {
        for (int i = 0; i < kTestElementCount; i += 4) {
            c_array.ReleaseIndex(i);
        }
        std::set<void *> visited;
        c_array.Visit<ComponentA>([&visited](ComponentA &a) { visited.insert(&a); });
        REQUIRE(visited.size() == kTestElementCount / 4 * 3);
    }
}

} // namespace MoonGlare::ECS