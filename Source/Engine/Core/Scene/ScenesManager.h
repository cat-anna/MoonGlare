/*
 * cSceneManager.h
 *
 *  Created on: 16-12-2013
 *      Author: Paweu
 */
#ifndef SCENESMANAGER_H_
#define SCENESMANAGER_H_

#include "Configuration.Scene.h"
#include <Renderer/StaticFog.h>
#include <Renderer/iAsyncLoader.h>

namespace MoonGlare::Core::Scene {

struct SceneDescriptor {
    StarVFS::FileID m_FID;
    std::string m_SID;
    std::mutex m_Lock;
    std::unique_ptr<ciScene> m_Ptr;

    Renderer::StaticFog staticFog;

    struct {
        bool m_SingleInstance;
        bool m_Valid;
        bool m_Loaded;
        bool m_LoadInProgress;
        bool m_Stateful;
        bool m_AllowMissingResources;
    } m_Flags;

    void DropScene();

    SceneDescriptor();
    ~SceneDescriptor();

    SceneDescriptor(const SceneDescriptor&) = delete;
    SceneDescriptor& operator=(const SceneDescriptor&) = delete;
};

using UniqueSceneDescriptor = std::unique_ptr<SceneDescriptor>;

enum class SceneChangeFence : uint32_t {
    Resources,
    ScriptThreads,
    MaxValue,
};

class ScenesManager {
public:
    ScenesManager();
    virtual ~ScenesManager();

    bool Initialize(World *world, const SceneConfiguration *configuration);
    bool Finalize();

    bool PostSystemInit();
    bool PreSystemStart();
    bool PreSystemShutdown();

    bool Step(const Core::MoveConfig &config);

    bool LoadScene(const std::string &SID); //async LoadNextScene
    bool DropSceneState(const std::string &SID);
    bool SetSceneStateful(const std::string &SID, bool value);
    const std::string& GetCurrentSceneName() const {
        ASSERT(m_CurrentSceneDescriptor);
        return m_CurrentSceneDescriptor->m_SID;
    }

    ciScene* CurrentScene() const { return m_CurrentScene; }
    void ChangeScene(const Core::MoveConfig & config);

    double GetSceneTime() const {
        if (!m_CurrentScene)
            return 0.0;
        return std::chrono::duration<double>(std::chrono::steady_clock::now() - sceneStartTime).count();
    }

    void SetSceneChangeFence(SceneChangeFence type, bool value);

#ifdef DEBUG_DUMP
    void DumpAllDescriptors(std::ostream& out);
#endif
    static void RegisterScriptApi(::ApiInitializer &api);
protected:
    using SceneSIDMap = std::unordered_map<string, SceneDescriptor*>;
    using SceneDescriptorTable = std::vector<UniqueSceneDescriptor>;

    std::atomic<uint32_t> changeSceneFences = 0;

    ciScene *m_CurrentScene = nullptr;
    std::chrono::steady_clock::time_point sceneStartTime;
    SceneDescriptor *m_CurrentSceneDescriptor = nullptr;
    SceneDescriptor *m_NextSceneDescriptor = nullptr;
    SceneDescriptor *m_LoadingSceneDescriptor = nullptr;

    std::recursive_mutex m_Lock;
    SceneSIDMap m_SIDMap;
    SceneDescriptorTable m_DescriptorTable;

    bool m_LoadingInProgress;
    World *m_World;
    const SceneConfiguration *sceneConfiguration = nullptr;
    Renderer::SharedAsyncLoaderObserver loaderObserver;

    bool LoadNextScene(const std::string &SID);					// load scene and execute it
    bool LoadNextScene(SceneDescriptor *descriptor);			// load scene and execute it
    bool LoadSceneData(SceneDescriptor *descriptor);			// load scene

    void ProcessPreviousScene(SceneDescriptor *descriptor);
    
    /** Manager shall be already locked */
    SceneDescriptor* FindDescriptor(const std::string &SID);
    SceneDescriptor* AllocDescriptor(StarVFS::FileID fid, const std::string &SID);
};

} //namespace Scene::Core::MoonGlare 

#endif //SCENEMANAGER_H_
