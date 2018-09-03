#pragma once 

#include <boost/noncopyable.hpp>

namespace MoonGlare::Core::Scene {

struct SceneConfiguration {
    std::string firstScene = "FirstScene";
    std::string loadingScene = "EngineLoadScene";
};

class iSceneManager : private boost::noncopyable {
public:
    virtual ~iSceneManager() {}

    virtual void Initialize(const SceneConfiguration *configuration) {}
    virtual void Finalize() {}
    virtual void PostSystemInit() {}
    virtual void PreSystemStart() {}
    virtual void PreSystemShutdown() {}

    virtual bool IsScenePending() const { return false; }
};

}
