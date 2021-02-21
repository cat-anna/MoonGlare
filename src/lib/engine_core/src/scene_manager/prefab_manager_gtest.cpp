#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "ecs/ecs_register.hpp"
#include "ecs/entity_manager_interface.hpp"
#include "scene_manager/prefab_manager.hpp"
#include <memory>

namespace MoonGlare::SceneManager {

namespace {

using namespace std::string_literals;

struct ComponentA {
    static constexpr Component::ComponentId kComponentId = 0;
    static constexpr char kComponentName[] = "a";
    int v = 1;
    bool operator==(const ComponentA &c) const { return v == c.v; }
};
void from_json(const nlohmann::json &j, ComponentA &p) {
    j.at("v").get_to(p.v);
}

struct ComponentB {
    static constexpr Component::ComponentId kComponentId = 1;
    static constexpr char kComponentName[] = "b";
    uint64_t v = 2;
    bool operator==(const ComponentB &c) const { return v == c.v; }
};
void from_json(const nlohmann::json &j, ComponentB &p) {
    j.at("v").get_to(p.v);
}

struct TestComponentRegister : public ECS::ECSRegister {
    TestComponentRegister() {
        RegisterComponent<ComponentA>();
        RegisterComponent<ComponentB>();
    }
};

constexpr auto kDisabledEntity = R"==({"enabled": false,"components": [],"children":[]})==";
constexpr auto kEmptyEntity = R"==({"enabled": true,"components": [],"children":[]})==";

const std::string kEmptyChildren =
    R"==({"enabled": true,"components": [],"children":[)=="s + kEmptyEntity + ","s + kEmptyEntity + R"==(]})=="s;

constexpr uint64_t kImportFileId = 10448041744059273782;
const auto kImported = R"==({"enabled": true,"import": 10448041744059273782})=="s;

const auto kDoubleImport =
    R"==({"enabled": true,"components": [],"children":[)=="s + kImported + ","s + kImported + R"==(]})=="s;

const auto kSimpleComponent = R"==({
    "enabled": true,
    "components": [
         {
            "enabled": true,
            "active": false,
            "class": "a",
            "data": { "v": 10 },
            "component_id": 0
        },
        {
            "enabled": false,
            "class": "b",
            "active": true,
            "data": { "v": 15 },
            "component_id": 1
        }
    ]
})=="s;

const auto kDubleComponent = R"==({
    "enabled": true,
    "components": [
         {
            "enabled": true,
            "active": true,
            "class": "a",
            "data": { "v": 11 },
            "component_id": 0
        },
        {
            "enabled": true,
            "active": true,
            "class": "b",
            "data": { "v": 16 },
            "component_id": 1
        }
    ]
})=="s;

const auto kMultipleComponents = R"==({
    "enabled": true,
    "components": [
        {
            "enabled": true,
            "active": false,
            "class": "a",
            "data": { "v": 7 },
            "component_id": 0
        }
    ],
    "children":[)=="s + kDubleComponent +
                                 ","s + kDubleComponent + R"==(]})=="s;

} // namespace

using namespace ::testing;

class PrefabManagerTest : public Test {
public:
    void SetUp() override {
        //
        entity_manager_mock.component_array = &component_array_mock;
        prefab_manager = std::make_unique<PrefabManager>(&fs_mock, &ecs_register, &ecs_register);
    }

    StrictMock<ReadOnlyFileSystemMock> fs_mock;
    StrictMock<ECS::EntityManagerMock> entity_manager_mock;
    StrictMock<ECS::ComponentArrayMock> component_array_mock;
    TestComponentRegister ecs_register;

    std::unique_ptr<PrefabManager> prefab_manager;
};

TEST_F(PrefabManagerTest, disabled_entity) {
    prefab_manager->LoadRootEntity(&entity_manager_mock, nlohmann::json::parse(kDisabledEntity));
}

TEST_F(PrefabManagerTest, empty_children) {
    prefab_manager->LoadRootEntity(&entity_manager_mock, nlohmann::json::parse(kEmptyChildren));

    EXPECT_THAT(entity_manager_mock.allocated_children.size(), Eq(2));
}

TEST_F(PrefabManagerTest, imported_child) {
    EXPECT_CALL(fs_mock, ReadFileByResourceId(kImportFileId, _)).WillOnce(Invoke([](auto, auto &data) {
        data = kEmptyEntity;
        return true;
    }));

    prefab_manager->LoadRootEntity(&entity_manager_mock, nlohmann::json::parse(kDoubleImport));

    EXPECT_THAT(entity_manager_mock.allocated_children.size(), Eq(2));
}

TEST_F(PrefabManagerTest, simple_import_component) {
    EXPECT_CALL(entity_manager_mock, GetEntityIndex(entity_manager_mock.root_entity, _))
        .WillRepeatedly(Invoke([](auto, auto &data) {
            data = 5;
            return true;
        }));

    ComponentA c_a;
    EXPECT_CALL(component_array_mock, CreateComponent(5, 0, false)).WillOnce(Return((void *)&c_a));
    EXPECT_CALL(component_array_mock, SetComponentActive(5, 0, false));

    prefab_manager->LoadRootEntity(&entity_manager_mock, nlohmann::json::parse(kSimpleComponent));

    EXPECT_THAT(entity_manager_mock.allocated_children.size(), Eq(0));
    EXPECT_THAT(c_a.v, Eq(10));
}

TEST_F(PrefabManagerTest, import_component) {
    std::vector<ComponentA> c_a(4);
    std::vector<ComponentB> c_b(4);

    EXPECT_CALL(component_array_mock, CreateComponent(_, 0, false)).WillRepeatedly(Invoke([&](auto id, auto, auto) {
        return (void *)(&c_a[id]);
    }));
    EXPECT_CALL(component_array_mock, CreateComponent(_, 1, false)).WillRepeatedly(Invoke([&](auto id, auto, auto) {
        return (void *)(&c_b[id]);
    }));

    EXPECT_CALL(component_array_mock, SetComponentActive(0, 0, false));

    prefab_manager->LoadRootEntity(&entity_manager_mock, nlohmann::json::parse(kMultipleComponents));

    EXPECT_THAT(entity_manager_mock.allocated_children.size(), Eq(2));

    std::vector<ComponentA> expected_a{{7}, {0xb}, {0xb}, {1}};
    EXPECT_THAT(c_a, ContainerEq(expected_a));

    std::vector<ComponentB> expected_b{{2}, {16}, {16}, {2}};
    EXPECT_THAT(c_b, ContainerEq(expected_b));
}

} // namespace MoonGlare::SceneManager
