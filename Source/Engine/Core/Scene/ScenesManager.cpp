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

    //struct {
        //bool m_SingleInstance;
        //bool m_Valid;
        //bool m_Loaded;
        //bool m_LoadInProgress;
        //bool m_Stateful;
        //bool m_AllowMissingResources;
    //} flags = { };
    //void DropScene() {
        //AddLogf(Debug, "Dropping scene: %s", m_SID.c_str());
        //m_Flags.m_Loaded = false;
        //if (m_Ptr) {
            //m_Ptr->Finalize();
            //m_Ptr.reset();
        //}
    //}
    //SceneDescriptor() { }
    //~SceneDescriptor() {
        //DropScene();
    //}
};

//----------------------------------------------------------------------------------

struct SceneInstance : private boost::noncopyable {
    Component::SubsystemManager subsystemManager;
    std::string sceneName;
    std::unordered_set<std::string> activeFences;
    SceneDescriptor *descriptor = nullptr;
    Component::EntityManager *entityManager = nullptr;
    Component::EventDispatcher *eventDispatcher = nullptr;
    Entity sceneRoot;

    bool isLoadingScene = false;

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
        //subsystemManager.GetEventDispatcher().Send<SceneStateChangeEvent>({ state, this });
    }

    void BeginScene() {
        SendState(SceneState::Started);
    }

    void EndScene() {
        SendState(SceneState::Paused);
    }

    bool Initialize(pugi::xml_node Node) {
        AddLog(Debug, "Initializing scene: " << sceneName);

        Entity worldRoot = entityManager->GetRootEntity();

        if (!entityManager->Allocate(worldRoot, sceneRoot, sceneName)) {
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

        SendState(SceneState::Created);

        EntityBuilder(&subsystemManager).Build(sceneRoot, Node.child("Entities"));
        return true;
    }

    void Finalize() {
        eventDispatcher->RemoveSubDispatcher(&subsystemManager.GetEventDispatcher());

        if (!subsystemManager.Finalize()) {
            AddLogf(Error, "Failed to finalize component manager");
        }
        entityManager->Release(sceneRoot);
    }

};

static constexpr std::string_view ResourceFenceName = ".Resources";

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
    CreateScene(event.sceneName);
    if (currentScene && !currentScene->isLoadingScene) {
        nextScene = loadingScene;
    }
}

void ScenesManager::HandleEvent(const SetSceneChangeFenceEvent &event) {
    UpdatePendingSceneFence(event.fence, event.active);
}

//----------------------------------------------------------------------------------

void ScenesManager::Step(const Core::MoveConfig & config) {
    if (nextScene) {
        ChangeScene();
    }

    if (currentScene)
        currentScene->Step(config);
}

void ScenesManager::ChangeScene() {
    if (!nextScene)
        return;

    if (currentScene) {
        currentScene->EndScene();
    } else {
        eventDispatcher->Queue(SetSceneEvent{ sceneConfiguration->firstScene });
    }

    auto *prevScene = currentScene;

    auto next = nextScene.load();
    nextScene = nullptr;

    currentScene = next;

    //sceneStartTime = std::chrono::steady_clock::now();

    if (currentScene) {
        currentScene->BeginScene();
        //config.deffered->SetStaticFog(m_CurrentSceneDescriptor->staticFog);
    }

    AddLogf(Hint, "Changed scene from '%s'[%p] to '%s'[%p]",
        (prevScene ? prevScene->sceneName.c_str() : "NULL"), prevScene,
        (next ? next->sceneName.c_str() : "NULL"), next);

    //if (prevSceneDesc)
        //ProcessPreviousScene(prevSceneDesc);
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
}

void ScenesManager::PreSystemStart() {
    loadingScene = CreateScene(sceneConfiguration->loadingScene);
    pendingScene = loadingScene;
    if (!pendingScene) {
        AddLogf(Error, "Cannot create loading scene");
        throw std::runtime_error("Cannot create loading scene");
    }
    pendingScene.load()->isLoadingScene = true;

    //__debugbreak();
    //std::this_thread::sleep_for(std::chrono::seconds(10));
    //currentScene = pendingScene;
    //pendingScene = nullptr;

    //if (!LoadNextScene(m_LoadingSceneDescriptor)) {
    //    AddLogf(Error, "Failed to load FallbackLoadScene");
    //    return false;
    //}
    //m_LoadingSceneDescriptor->m_Flags.m_AllowMissingResources = true;
    //m_LoadingSceneDescriptor->m_Flags.m_Stateful = true;
    //JobQueue::QueueJob([this] {
    //    std::this_thread::sleep_for(std::chrono::seconds(1));
    //    while (m_NextSceneDescriptor)
    //        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    //    LoadNextScene(sceneConfiguration->firstScene);
    //});
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

    auto sceneuptr = std::make_unique<SceneInstance>();
    auto &scene = *sceneuptr;
    scene.sceneName = sceneName;
    scene.descriptor = desc;
    scene.entityManager = interfaceMap.GetInterface<Component::EntityManager>();
    scene.eventDispatcher = eventDispatcher;
    pendingScene = &scene;
    assert(scene.entityManager);
    if (!LoadSceneData(desc, &scene)) {
        AddLogf(Error, "Failed to load scene '%s' from descriptor '%s'", sceneName.c_str(), descName.c_str());
        return nullptr;
    }
    sceneInstances[sceneName] = std::move(sceneuptr);
   
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

    //sd->m_Flags.m_Valid = false;
    //sd->m_Flags.m_SingleInstance = false;

    return rawptr;
}

SceneDescriptor* ScenesManager::FindDescriptor(const std::string &Name) {
    std::lock_guard<std::recursive_mutex> lock(mutex);
    auto sidit = descriptorsByName.find(Name);
    if (sidit != descriptorsByName.end()) {
        AddLogf(Debug, "Scene exits: sid:%s", Name.c_str());
        return sidit->second;
    }
    AddLogf(Debug, "Scene DOES NOT exits: sid:%s", Name.c_str());
    return nullptr;
}

//----------------------------------------------------------------------------------

void ScenesManager::UpdatePendingSceneFence(const std::string_view fenceName, bool state) {
    auto pending = pendingScene.load();
    if (pending) {
        bool ready = pending->SetFenceState(std::string(fenceName), state);
        if (ready) {
            pendingScene = nullptr;
            nextScene = pending;
        }
    }
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

    //descriptor->m_Flags.m_Stateful = sc.m_Stateful;
    //descriptor->staticFog = sc.m_StaticFog;

    if (!instance->Initialize(xmlroot)) {
        AddLogf(Error, "Failed to initialize scene '%s'", instance->sceneName.c_str());
        return false;
    }

    //instance->subsystemManager.GetEventDispatcher().Register<SetSceneEvent>(this);
    return true;
}

//----------------------------------------------------------------------------------

#if 0

//void ScenesManager::SetSceneChangeFence(SceneChangeFence type, bool value) {
    //uint32_t bit = 1 << static_cast<uint32_t>(type);
    //if (value) {
        //changeSceneFences |= bit;
        //DebugLogf(Hint, "Added scene change fence '%s' -> 0x%x", SceneChangeFenceNames[type], changeSceneFences.load());
    //}                                                                                 
    //else {
        //changeSceneFences &= ~bit;
        //DebugLogf(Hint, "Removed scene change fence '%s' -> 0x%x", SceneChangeFenceNames[type], changeSceneFences.load());
    //}
//}
                              
//----------------------------------------------------------------------------------

bool ScenesManager::LoadScene(const std::string &SID) {
    ASSERT(m_LoadingSceneDescriptor);

    auto sd = FindDescriptor(SID);
    if (!sd) {
        AddLogf(Error, "Scene '%s' does not exits", SID.c_str());
        return false;
    }

    m_NextSceneDescriptor = m_LoadingSceneDescriptor;

    JobQueue::QueueJob([this, sd] {
        while (m_NextSceneDescriptor)
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        LoadNextScene(sd);
    });

    return true;
}

bool ScenesManager::LoadNextScene(const std::string &SID) {
    auto d = FindDescriptor(SID);
    if (!d) {
        AddLogf(Error, "Scene '%s' does not exits", SID.c_str());
        return false;
    }
    return LoadNextScene(d);
}

bool ScenesManager::LoadNextScene(SceneDescriptor *descriptor) {
    assert(descriptor);
    if (!LoadSceneData(descriptor)) {
        AddLogf(Error, "Failed to load scene '%s'", descriptor->m_SID.c_str());
        return false;
    }

    m_NextSceneDescriptor = descriptor;
    return true;
}



void ScenesManager::ProcessPreviousScene(SceneDescriptor *descriptor) {
    ASSERT(descriptor);
    if (descriptor->m_Flags.m_Stateful) {
        return;
    }

    descriptor->DropScene();
}

//----------------------------------------------------------------------------------

bool ScenesManager::DropSceneState(const std::string &SID) {
    auto sd = FindDescriptor(SID);
    if (!sd) {
        AddLogf(Error, "There is no scene: %s", SID.c_str());
        return false;
    }

    if (!sd->m_Flags.m_Loaded) {
        AddLogf(Warning, "Scene '%s' is not loaded!", SID.c_str());
        return false;
    }

    if (sd == m_CurrentSceneDescriptor) {
        AddLogf(Warning, "Cannot drop current scene: %s", SID.c_str());
        return false;
    }

    sd->DropScene();
    AddLogf(Info, "Dropped scene state: %s", SID.c_str());

    return true;
}

bool ScenesManager::SetSceneStateful(const std::string & SID, bool value) {
    auto sd = FindDescriptor(SID);
    if (!sd) {
        AddLogf(Error, "There is no scene: %s", SID.c_str());
        return false;
    }

    if (!sd->m_Flags.m_Loaded) {
        AddLogf(Warning, "Scene '%s' is not loaded!", SID.c_str());
        return false;
    }

    sd->m_Flags.m_Stateful = value;

    return false;
}

//----------------------------------------------------------------------------------

#ifdef DEBUG_DUMP

void ScenesManager::DumpAllDescriptors(std::ostream& out) {
    out << "Scene descriptors:\n";
    for (auto &it : m_DescriptorTable) {
        auto &sd = *it;
        char buf[128];
        sprintf(buf, "%20s ->%5u[%s]\n", sd.m_SID.c_str(), (unsigned)sd.m_FID, GetFileSystem()->GetFullFileName(sd.m_FID).c_str());
        out << buf;
    }
    out << "\n";
}

#endif // DEBUG_DUMP

//----------------------------------------------------------------------------------

#endif

} //namespace Scene::Core::MoonGlare 
