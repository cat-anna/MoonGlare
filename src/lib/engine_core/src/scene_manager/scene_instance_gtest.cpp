#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "ecs/ecs_register.hpp"
#include "scene_instance.hpp"
#include "scene_manager/configuration.hpp"
#include <memory>

namespace MoonGlare::SceneManager {

using namespace ::testing;

struct TestComponentRegister : public ECS::ECSRegister {
    TestComponentRegister() {}
};

class SceneInstanceTest : public Test {
public:
    void Start() {
        EXPECT_CALL(async_loader_mock, LoadFile(resource_id, _)).WillOnce(Invoke([this](auto id, auto functor) {
            functor(id, scene_file_data);
        }));

        EXPECT_CALL(prefab_manager_mock, LoadSystemConfiguration(_, _));
        EXPECT_CALL(prefab_manager_mock, LoadRootEntity(_, _));

        scene_instance = std::make_unique<SceneInstance>(scene_name, resource_id, scene_id, &async_loader_mock,
                                                         &ecs_register, &prefab_manager_mock);
    }

    // SceneInstanceFactoryMock *factory_mock = nullptr;
    StrictMock<AsyncLoaderMock> async_loader_mock;
    StrictMock<PrefabManagerMock> prefab_manager_mock;
    TestComponentRegister ecs_register;

    std::string scene_name = "test_scene";
    FileResourceId resource_id = 5;
    ECS::EntityManagerId scene_id = 2;
    std::string scene_file_data = R"==({"entity":{},"systems":[]})==";

    std::unique_ptr<SceneInstance> scene_instance;
};

TEST_F(SceneInstanceTest, basic_construct) {
    Start();
    EXPECT_TRUE(scene_instance->ReadyForActivation());
}

} // namespace MoonGlare::SceneManager