#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "scene_manager/configuration.hpp"
#include "scene_manager/scenes_manager.hpp"
#include <memory>

namespace MoonGlare::SceneManager {

using namespace ::testing;

class SceneManagerTest : public Test {
public:
    void SetUp() override {
        EXPECT_CALL(fs_mock, FindFilesByExt(_, _)).WillOnce(Invoke([](auto, auto &table) {
            table.clear();
            table.emplace_back(FileInfoTable::value_type{"test1.sdx", false, false, 1, 1, 1});
            table.emplace_back(FileInfoTable::value_type{"test2.sdx", false, false, 2, 2, 2});
            return true;
        }));

        auto factory = std::make_unique<StrictMock<SceneInstanceFactoryMock>>();
        factory_mock = factory.get();
        scene_manager = std::make_unique<ScenesManager>(&fs_mock, std::move(factory));
    }

    SceneInstanceFactoryMock *factory_mock = nullptr;
    StrictMock<ReadOnlyFileSystemMock> fs_mock;

    std::unique_ptr<ScenesManager> scene_manager;
};

TEST_F(SceneManagerTest, load_and_activate) {
    SceneInstanceMock *scene_mock = nullptr;

    EXPECT_CALL(*factory_mock, CreateSceneInstance(kLoadingSceneName, 1))
        .WillOnce(Invoke([&](auto, auto) {
            auto uptr = std::make_unique<StrictMock<SceneInstanceMock>>(kLoadingSceneName);
            scene_mock = uptr.get();
            return uptr;
        }));

    auto loading_scene = scene_manager->CreateScene("test1", kLoadingSceneName);
    bool scene_ready = false;

    EXPECT_CALL(*scene_mock, DoStep(_)).Times(AnyNumber());

    for (int i = 0; i < 50; ++i) {
        scene_mock->scene_ready = i > 10;
        scene_manager->DoStep(0.1);
    }

    //TODO: no side effect yet
}

} // namespace MoonGlare::SceneManager