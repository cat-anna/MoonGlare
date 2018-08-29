#include <pch.h>

#include <libSpace/src/Container/EnumMapper.h>

#include <Foundation/iFileSystem.h>

#include <Foundation/Component/EventDispatcher.h>
#include <Foundation/Component/EntityManager.h>

#include <nfMoonGlare.h>
#include "ScenesManager.h"
#include <Engine/Core/Engine.h>

#include "../Component/SubsystemManager.h"

#include <Renderer/Deferred/DeferredFrontend.h>
#include <Core/Scripts/LuaApi.h>
#include <Core/EntityBuilder.h>

#include <Math.x2c.h>
#include <StaticFog.x2c.h>
#include <Scene.x2c.h>

#include "Scene.Events.h"

namespace MoonGlare::Core::Scene {

struct SceneDescriptor : private boost::noncopyable {
    StarVFS::FileID fileId;
    std::string Name;
};

//----------------------------------------------------------------------------------

struct SceneInstance : private boost::noncopyable {
    using TimePoint = Component::SubsystemUpdateData::TimePoint;

    Component::SubsystemManager subsystemManager;

    const std::string sceneName;
    TimePoint localTimeStart = TimePoint::clock::now();
    double localTimeBase = 0.0;

    bool wantsToSleep = false;

    SceneDescriptor *descriptor = nullptr;
    std::unordered_set<std::string> activeFences;
    Component::EntityManager *entityManager = nullptr;
    Component::EventDispatcher *eventDispatcher = nullptr;
    MoonGlare::Renderer::StaticFog staticFog = { };
    Entity sceneRoot;

    SceneInstance(std::string name) : sceneName(std::move(name)) { }

    void PauseTime(Component::SubsystemUpdateData &sud, TimePoint timePoint) {
        localTimeBase += TimeDiff(localTimeStart, timePoint);
        localTimeStart = timePoint;
    }
    void RestoreTime(Component::SubsystemUpdateData &sud, TimePoint timePoint) {
        localTimeStart = timePoint;
        sud.localTimeBase = localTimeBase;
        sud.localTimeStart = timePoint;
    }

    //returns true if NO fence is set 
    bool SetFenceState(std::string name, bool state) {
        if (!name.empty()) {
            if (state)
                activeFences.emplace(std::move(name));
            else
                activeFences.erase(std::move(name));
        }
        return activeFences.empty();
    }      

    void Step(const MoveConfig &conf) {
        subsystemManager.Step(conf);
    }

    void SendState(SceneState state) {
        SceneStateChangeEvent ssce;
        ssce.sceneName = sceneName;
        ssce.state = state;
        eventDispatcher->Queue<SceneStateChangeEvent>(ssce);
    }

    bool Initialize(pugi::xml_node Node) {
        AddLog(Debug, "Initializing scene: " << sceneName);

        if (!entityManager->Allocate(sceneRoot, sceneName)) {
            AddLogf(Error, "Failed to allocate scene entity!");
            return false;
        }

        if (!subsystemManager.LoadComponents(Node.child("Components"))) {
            AddLogf(Error, "Failed to load components");
            return false;
        }

        if (!subsystemManager.Initialize(sceneRoot)) {
            AddLogf(Error, "Failed to initialize component manager");
            return false;
        }

        eventDispatcher->AddSubDispatcher(&subsystemManager.GetEventDispatcher());

        EntityBuilder(&subsystemManager).Build(sceneRoot, Node.child("Entities"));
        return true;
    }

    void Finalize() {
        AddLog(Debug, "Finalizing scene: " << sceneName);
        eventDispatcher->RemoveSubDispatcher(&subsystemManager.GetEventDispatcher());

        if (!subsystemManager.Finalize()) {
            AddLogf(Error, "Failed to finalize component manager");
        }
        entityManager->Release(sceneRoot);
    }
};

static constexpr std::string_view ResourceFenceName = "system.resources";

//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------

RegisterApiBaseClass(ScenesManager, &ScenesManager::RegisterScriptApi);

ScenesManager::ScenesManager(InterfaceMap &ifaceMap) : interfaceMap(ifaceMap) {
}

ScenesManager::~ScenesManager() {
}

//----------------------------------------------------------------------------------

void ScenesManager::RegisterScriptApi(ApiInitializer &api) {
    api
        //.beginClass<ScenesManager>("cScenesManager")
        //    .addFunction("LoadScene", &ScenesManager::LoadScene)
        //    .addFunction("DropSceneState", &ScenesManager::DropSceneState)
        //.endClass()
        .beginClass<SceneConfiguration>("SceneConfiguration")
            .addData("firstScene", &SceneConfiguration::firstScene, true)
            .addData("loadingScene", &SceneConfiguration::loadingScene, true)
        .endClass()
        ;
}

//----------------------------------------------------------------------------------

void ScenesManager::Initialize(const SceneConfiguration *configuration) {
    assert(configuration);
    sceneConfiguration = configuration;

    eventDispatcher = interfaceMap.GetInterface<Component::EventDispatcher>();
    assert(eventDispatcher);

    eventDispatcher->Register<Renderer::RendererResourceLoaderEvent>(this);
    eventDispatcher->Register<SetSceneEvent>(this);
    eventDispatcher->Register<SetSceneChangeFenceEvent>(this);
}

void ScenesManager::Finalize() {
}

//----------------------------------------------------------------------------------

void ScenesManager::HandleEvent(const Renderer::RendererResourceLoaderEvent &event) {
    UpdatePendingSceneFence(ResourceFenceName, event.busy);
}

void ScenesManager::HandleEvent(const SetSceneEvent &event) {
    std::lock_guard<std::recursive_mutex> lock(mutex);
    if (currentScene)
        currentScene->wantsToSleep = event.suspendCurrent;
    
    if (SceneInstance *inst = FindSceneInstance(event.sceneName); inst) {
        if (inst->activeFences.empty()) {
            nextScene = inst;
            pendingScene = nullptr;
        } else {
            nextScene = loadingScene;
            pendingScene = inst;
        }
        return;
    }

    CreateScene(event.GetTypeName(), event.sceneName);
}

void ScenesManager::HandleEvent(const SetSceneChangeFenceEvent &event) {
    std::string fenceName = "script." + std::string(event.fence);
    std::lock_guard<std::recursive_mutex> lock(mutex);
    if (!event.sceneName.empty()) {
        if (SceneInstance *inst = FindSceneInstance(event.sceneName); inst) {
            if (inst == currentScene)
                return; //TODO: decide how to properly handle this case

            if (inst == nextScene) {
                nextScene = loadingScene;
                pendingScene = inst;
                return;
            }
            if (inst == pendingScene)
                UpdatePendingSceneFence(fenceName, event.active);
            else
                inst->SetFenceState(fenceName, event.active);
        }
    } else
        UpdatePendingSceneFence(fenceName, event.active);
}

//----------------------------------------------------------------------------------

void ScenesManager::Step(Core::MoveConfig & config) {
    if (nextScene) {
        ChangeScene(config);
    }

    if (currentScene)
        currentScene->Step(config);
}

void ScenesManager::ChangeScene(Core::MoveConfig & config) {
    if (!nextScene)
        return;

    std::lock_guard<std::recursive_mutex> lock(mutex);

    SceneInstance::TimePoint currentTime = SceneInstance::TimePoint::clock::now();

    if (currentScene) {
        currentScene->PauseTime(config, currentTime);
        currentScene->SendState(SceneState::Paused);
    } else {
        eventDispatcher->Queue(SetSceneEvent( sceneConfiguration->firstScene.c_str()));
    }
                                             
    auto *prevScene = currentScene;

    auto next = nextScene.load();
    nextScene = nullptr;

    currentScene = next;

    AddLogf(Hint, "Changed scene from '%s'[%p] to '%s'[%p]",
        (prevScene ? prevScene->sceneName.c_str() : "NULL"), prevScene,
        (next ? next->sceneName.c_str() : "NULL"), next);

    ProcessPreviousScene(prevScene);

    if (currentScene) {
        currentScene->RestoreTime(config, currentTime);
        currentScene->SendState(SceneState::Started);
        config.deffered->SetStaticFog(currentScene->staticFog);
    }
}

//----------------------------------------------------------------------------------

void ScenesManager::PostSystemInit() {
    auto fs = interfaceMap.GetInterface<iFileSystem>();
    assert(fs);
    StarVFS::DynamicFIDTable scenefids;
    fs->FindFilesByExt(Configuration::SceneManager::SceneFileExt, scenefids);

    for (auto fileid : scenefids) {
        std::string fileName = fs->GetFileName(fileid);

        while (fileName.back() != '.')
            fileName.pop_back();
        fileName.pop_back();

        AddLogf(Debug, "Found scene: %s -> %s", fs->GetFullFileName(fileid).c_str(), fileName.c_str());

        CreateDescriptor(fileid, fileName);
    }
#ifdef DEBUG_DUMP
    DumpAllDescriptors();
#endif
}

void ScenesManager::PreSystemStart() {
    loadingScene = CreateScene(sceneConfiguration->loadingScene, "LoadingScene");
    if (!loadingScene) {
        AddLogf(Error, "Cannot create loading scene");
        throw std::runtime_error("Cannot create loading scene");
    }
}

//----------------------------------------------------------------------------------

SceneInstance* ScenesManager::CreateScene(const std::string &descName, const std::string &alias) {
    std::string sceneName = alias.empty() ? descName : alias;
    std::lock_guard<std::recursive_mutex> lock(mutex);

    if (auto *inst = FindSceneInstance(sceneName); inst) {
        AddLogf(Error, "Scene '%s' instance does exits. Cannot create new one.", descName.c_str());
        return nullptr;
    }

    auto *desc = FindDescriptor(descName);
    if (!desc) {
        AddLogf(Error, "Scene '%s' does not exit. Cannot create new one.", descName.c_str());
        return nullptr;
    }

    auto sceneuptr = std::make_unique<SceneInstance>(sceneName);
    auto &scene = *sceneuptr;
    scene.descriptor = desc;
    scene.entityManager = interfaceMap.GetInterface<Component::EntityManager>();
    scene.eventDispatcher = eventDispatcher;
    pendingScene = &scene;

    assert(scene.entityManager);
    if (!LoadSceneData(desc, &scene)) {
        AddLogf(Error, "Failed to load scene '%s' from descriptor '%s'", sceneName.c_str(), descName.c_str());
        pendingScene = nullptr;
        return nullptr;
    }

    sceneInstances[sceneName] = std::move(sceneuptr);

    scene.SendState(SceneState::Created);

    if (currentScene != loadingScene) {
        nextScene = loadingScene;
    }
    return &scene;
}

SceneInstance* ScenesManager::FindSceneInstance(const std::string &sceneName) {
    std::lock_guard<std::recursive_mutex> lock(mutex);
    auto sidit = sceneInstances.find(sceneName);
    if (sidit != sceneInstances.end()) {
        AddLogf(Debug, "Scene instance exits: sid:%s", sceneName.c_str());
        return sidit->second.get();
    }
    AddLogf(Debug, "Scene instance DOES NOT exits: sid:%s", sceneName.c_str());
    return nullptr;
}

//----------------------------------------------------------------------------------

SceneDescriptor* ScenesManager::CreateDescriptor(StarVFS::FileID fid, const std::string &Name) {
    std::lock_guard<std::recursive_mutex> lock(mutex);

    if (auto ptr = FindDescriptor(Name); ptr) {
        AddLogf(Error, "Cannot allocate scene, SID already exists. fid:%u sid:%s", (unsigned)fid, Name.c_str());
        return nullptr;
    }

    auto desc = std::make_unique<SceneDescriptor>();
    auto rawptr = desc.get();
    descriptorsByName[Name] = desc.get();

    desc->Name = Name;
    desc->fileId = fid;

    knownSceneDescriptors.emplace_back(std::move(desc));

    return rawptr;
}

SceneDescriptor* ScenesManager::FindDescriptor(const std::string &Name) {
    std::lock_guard<std::recursive_mutex> lock(mutex);
    auto sidit = descriptorsByName.find(Name);
    if (sidit != descriptorsByName.end()) {
        AddLogf(Debug, "Scene exists: sid:%s", Name.c_str());
        return sidit->second;
    }
    AddLogf(Debug, "Scene DOES NOT exists: sid:%s", Name.c_str());
    return nullptr;
}

//----------------------------------------------------------------------------------

void ScenesManager::UpdatePendingSceneFence(const std::string_view fenceName, bool state) {
    std::lock_guard<std::recursive_mutex> lock(mutex);
    auto pending = pendingScene.load();
    if (pending) {
        bool ready = pending->SetFenceState(std::string(fenceName), state);
        if (ready) {
            pendingScene = nullptr;
            nextScene = pending;
        }
    }
}

void ScenesManager::ProcessPreviousScene(SceneInstance *instance) {
    if (!instance)
        return;

    if (instance == loadingScene)
        return;

    if (instance->wantsToSleep)
        //TODO: some standby mechanics
        return;

    DropSceneInstance(instance);
}

void ScenesManager::DropSceneInstance(SceneInstance *instance) {
    if (!instance)
        return;

    std::lock_guard<std::recursive_mutex> lock(mutex);
    instance->SendState(SceneState::BeforeDestruction);
    instance->Finalize();
    sceneInstances.erase(instance->sceneName);
}

bool ScenesManager::LoadSceneData(SceneDescriptor *descriptor, SceneInstance* instance) {
    assert(descriptor);
    assert(instance);

    AddLogf(Debug, "Loading scene '%s'", instance->sceneName.c_str());

    auto fs = interfaceMap.GetInterface<iFileSystem>();
    assert(fs);

    XMLFile doc;
    if (!fs->OpenXML(doc, descriptor->fileId)) {
        AddLogf(Warning, "Unable to load xml for scene: '%s'", instance->sceneName.c_str());
        return false;
    }

    auto xmlroot = doc->document_element();

    x2c::Core::Scene::SceneConfiguration_t sc;
    sc.ResetToDefault();
    if (!sc.Read(xmlroot, "Configuration")) {
        AddLogf(Error, "Failed to read configuration for scene: %s", instance->sceneName.c_str());
        return false;
    }

    instance->staticFog = sc.m_StaticFog;

    if (!instance->Initialize(xmlroot)) {
        AddLogf(Error, "Failed to initialize scene '%s'", instance->sceneName.c_str());
        return false;
    }

    return true;
}

//----------------------------------------------------------------------------------

void ScenesManager::DumpAllDescriptors() {
    std::stringstream out;
    auto fs = interfaceMap.GetInterface<iFileSystem>();
    assert(fs);
    out << "Scene descriptors:\n";
    for (auto &it : knownSceneDescriptors) {
        auto &sd = *it;
        char buf[128];
        sprintf(buf, "%20s ->%5u[%s]\n", sd.Name.c_str(), (unsigned)sd.fileId, fs->GetFullFileName(sd.fileId).c_str());
        out << buf;
    }
    out << "\n";

    AddLog(Debug, out.str());
}

//----------------------------------------------------------------------------------

} //namespace Scene::Core::MoonGlare 
