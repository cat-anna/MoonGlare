#pragma once

#include <memory>
#include <resource_id.hpp>
#include <stepable_interface.hpp>
#include <string>
#include <string_view>

namespace MoonGlare::SceneManager {

class iSceneInstance : public iStepableObject {
public:
    virtual ~iSceneInstance() = default;
    iSceneInstance(std::string _scene_name) : scene_name(std::move(_scene_name)) {}

    virtual bool ReadyForActivation() const = 0;

    const std::string GetSceneName() const { return scene_name; }

private:
    const std::string scene_name;
};

#ifdef WANTS_GTEST_MOCKS
struct SceneInstanceMock : public iSceneInstance {
    bool scene_ready = false;
    SceneInstanceMock(const char *_name = "SceneInstanceMock") : iSceneInstance(_name) {
        EXPECT_CALL(*this, ReadyForActivation()).WillRepeatedly(::testing::Invoke([this]() {
            return scene_ready;
        }));
    }
    MOCK_CONST_METHOD0(ReadyForActivation, bool());
    MOCK_METHOD1(DoStep, void(double));
};
#endif

/*@ [LuaScenesManagerModule/ScenesManager] ScenesManager
    TODO
@*/
class iScenesManager {
public:
    virtual ~iScenesManager() = default;

    /*@ [ScenesManager/_] ScenesManager:create_scene(scene_resource, scene_name)
    TODO
@*/
    virtual iSceneInstance *CreateScene(const std::string &resource_name,
                                        std::string scene_name) = 0;
    // virtual std::shared_ptr<iScene> FindScene(std::string_view scene_name) = 0;

    // virtual void SetCurrentScene(std::string_view scene_name) = 0;

    // virtual void DiscardScene(std::string_view scene_name) = 0;

    //old
    // virtual void Initialize(const SceneConfiguration *configuration) {}
    // virtual void Finalize() {}
    // virtual void PostSystemInit() {}
    // virtual void PreSystemStart() {}
    // virtual void PreSystemShutdown() {}
    // virtual bool IsScenePending() const { return false; }
};

class iSceneInstanceFactory {
public:
    virtual ~iSceneInstanceFactory() = default;

    virtual std::unique_ptr<iSceneInstance> CreateSceneInstance(std::string scene_name,
                                                                FileResourceId res_id) = 0;
};

#ifdef WANTS_GTEST_MOCKS
struct SceneInstanceFactoryMock : public iSceneInstanceFactory {
    MOCK_METHOD2(CreateSceneInstance, std::unique_ptr<iSceneInstance>(std::string, FileResourceId));
};
#endif

} // namespace MoonGlare::SceneManager
