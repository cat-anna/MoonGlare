#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "ecs/entity_manager.hpp"
#include <set>

namespace MoonGlare::ECS {

using namespace ::testing;

class EntityManagerTest : public Test {
public:
    StrictMock<ComponentArrayMock> component_array_mock;
    std::unique_ptr<EntityManager> em;

    void SetUp() override {
        EXPECT_CALL(component_array_mock, MarkIndexAsValid(0));
        em = std::make_unique<EntityManager>(1, &component_array_mock);
    };
};

TEST_F(EntityManagerTest, root_cannot_be_released) {

    auto root_e = em->GetRootEntity();
    EXPECT_THAT(root_e, Ne(0));
    EXPECT_TRUE(em->IsValid(root_e));
    em->Release(root_e);
    EXPECT_TRUE(em->IsValid(root_e));
}

TEST_F(EntityManagerTest, entity_can_be_released) {
    EXPECT_CALL(component_array_mock, MarkIndexAsValid(1));
    auto e0 = em->NewEntity();
    EXPECT_THAT(e0, Ne(0));
    EXPECT_TRUE(em->IsValid(e0));
    EXPECT_CALL(component_array_mock, ReleaseIndex(1, true));
    em->Release(e0);
    EXPECT_FALSE(em->IsValid(e0));
}

TEST_F(EntityManagerTest, child_handle_not_valid_after_parent_is_released) {

    EXPECT_CALL(component_array_mock, MarkIndexAsValid(1));
    EXPECT_CALL(component_array_mock, MarkIndexAsValid(2));

    auto parent = em->NewEntity();
    auto child0 = em->NewEntity(parent);

    EXPECT_CALL(component_array_mock, ReleaseIndex(1, true));
    EXPECT_CALL(component_array_mock, ReleaseIndex(2, true));

    EXPECT_THAT(parent, Ne(0));
    EXPECT_THAT(child0, Ne(0));
    EXPECT_THAT(child0, Ne(parent));

    EXPECT_TRUE(em->IsValid(parent));
    EXPECT_TRUE(em->IsValid(child0));
    em->Release(parent);
    EXPECT_FALSE(em->IsValid(parent));
    EXPECT_FALSE(em->IsValid(child0));
}

} // namespace MoonGlare::ECS