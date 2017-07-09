/*
 * ScenesManager.cpp
 *
 *  Created on: 16-12-2013
 *      Author: Paweu
 */
#include <pch.h>
#include <MoonGlare.h>
#include "ScenesManager.h"
#include <Engine/Core/Engine.h>

#include <Renderer/Renderer.h>

#include <Scene.x2c.h>

namespace MoonGlare::Core::Scene {

SceneDescriptor::SceneDescriptor() : m_Ptr() {
    memset(&m_Flags, 0, sizeof(m_Flags));
}

SceneDescriptor::~SceneDescriptor() {
    DropScene();
}

void SceneDescriptor::DropScene() {
    AddLogf(Debug, "Dropping scene: %s", m_SID.c_str());
    m_Flags.m_Loaded = false;
    if (m_Ptr) {
        m_Ptr->Finalize();
        m_Ptr.reset();
    }
}

//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------

SPACERTTI_IMPLEMENT_STATIC_CLASS(ScenesManager)
RegisterApiBaseClass(ScenesManager, &ScenesManager::RegisterScriptApi);
ScenesManager* GetScenesManager() { return GetEngine()->GetWorld()->GetScenesManager(); }
RegisterApiInstance(ScenesManager, &GetScenesManager, "SceneManager");

ScenesManager::ScenesManager()
    : m_LoadingSceneDescriptor(nullptr)
    , m_CurrentScene(nullptr)
    , m_CurrentSceneDescriptor(nullptr)
    , m_World(nullptr)
    , m_LoadingInProgress(false)
{
}

ScenesManager::~ScenesManager() {
}

//----------------------------------------------------------------------------------

void ScenesManager::RegisterScriptApi(ApiInitializer &api) {
    api
        .beginClass<ThisClass>("cScenesManager")
            .addFunction("LoadScene", &ThisClass::LoadScene)
            .addFunction("DropSceneState", &ThisClass::DropSceneState)
            .addFunction("CurrentSceneName", &ThisClass::GetCurrentSceneName)
        .endClass()
        .beginClass<SceneConfiguration>("cSceneConfiguration")
            .addData("firstScene", &SceneConfiguration::firstScene, true)
            .addData("loadingScene", &SceneConfiguration::loadingScene, true)
        .endClass()
        ;
}

//----------------------------------------------------------------------------------

bool ScenesManager::Initialize(World *world, const SceneConfiguration *configuration) {
    ASSERT(world);
    ASSERT(configuration);

    m_World = world;
    m_DescriptorTable.reserve(32);//TODO: some config?
    sceneConfiguration = configuration;

    return true;
}

bool ScenesManager::Finalize() { 
    m_SIDMap.clear();
    m_DescriptorTable.clear();
    return true;
}

bool ScenesManager::PostSystemInit() {
    auto fs = GetFileSystem();
    StarVFS::DynamicFIDTable scenefids;
    fs->FindFiles(Configuration::SceneManager::SceneFileExt, scenefids);

    for (auto fid : scenefids) {
        std::string sid = fs->GetFileName(fid);

        while (sid.back() != '.')
            sid.pop_back();
        sid.pop_back();

        AddLogf(Debug, "Found scene: %s -> %s", fs->GetFullFileName(fid).c_str(), sid.c_str());
            
        AllocDescriptor(fid, sid);
    }
    
    return true;
}

bool ScenesManager::PreSystemStart() {
    m_LoadingSceneDescriptor = FindDescriptor(sceneConfiguration->loadingScene);
    if (!m_LoadingSceneDescriptor) {
        AddLogf(Error, "There is no FallbackLoadScene");
        return false;
    }

    if (!LoadNextScene(m_LoadingSceneDescriptor)) {
        AddLogf(Error, "Failed to load FallbackLoadScene");
        return false;
    }

    m_LoadingSceneDescriptor->m_Flags.m_AllowMissingResources = true;
    m_LoadingSceneDescriptor->m_Flags.m_Stateful = true;

    JobQueue::QueueJob([this] { 
        while(m_NextSceneDescriptor)
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        LoadNextScene(sceneConfiguration->firstScene);
    });

    return true;
}

bool ScenesManager::PreSystemShutdown() {
    return true;
}

//----------------------------------------------------------------------------------

void ScenesManager::ChangeScene() {
    if (!m_NextSceneDescriptor)
        return;

    if (m_CurrentScene) {
        if (!m_NextSceneDescriptor->m_Flags.m_AllowMissingResources) {
            if (!m_World->GetRendererFacade()->AllResourcesLoaded()) {
                return;
            }
        }
        m_CurrentScene->EndScene();
    }
    else {
        if (!m_World->GetRendererFacade()->AllResourcesLoaded()) {
            return;
        }
    }

    auto *prevScene = m_CurrentScene;
    auto *prevSceneDesc = m_CurrentSceneDescriptor;

    m_CurrentScene = m_NextSceneDescriptor->m_Ptr.get();
    m_CurrentSceneDescriptor = m_NextSceneDescriptor;
    m_NextSceneDescriptor = nullptr;
    sceneStartTime = std::chrono::steady_clock::now();

    if (m_CurrentScene) {
        m_CurrentScene->BeginScene();
    }

    AddLogf(Hint, "Changed scene from '%s'[%p] to '%s'[%p]",
        (prevSceneDesc ? prevSceneDesc->m_SID.c_str() : "NULL"), prevScene,
        (m_CurrentScene ? m_CurrentSceneDescriptor->m_SID.c_str() : "NULL"), m_CurrentScene);

    if (prevSceneDesc)
        ProcessPreviousScene(prevSceneDesc);
}

bool ScenesManager::Step(const Core::MoveConfig & config) {
    if (m_NextSceneDescriptor) {
        ChangeScene();
    }

    if (m_CurrentScene)
        m_CurrentScene->DoMove(config);
    return true;
}

//----------------------------------------------------------------------------------

bool ScenesManager::LoadScene(const std::string &SID) {
    ASSERT(m_LoadingSceneDescriptor);

    auto sd = FindDescriptor(SID);
    if (!sd) {
        AddLogf(Error, "Scene '%s' does not exits", SID.c_str());
        return false;
    }

    SetNextSceneDescriptor(m_LoadingSceneDescriptor);

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
    ASSERT(descriptor);
    if (!LoadSceneData(descriptor)) {
        AddLogf(Error, "Failed to load scene '%s'", descriptor->m_SID.c_str());
        return false;
    }

    return SetNextSceneDescriptor(descriptor);
}

bool ScenesManager::LoadSceneData(SceneDescriptor *descriptor) {
    ASSERT(descriptor);

    LOCK_MUTEX(descriptor->m_Lock);

    if (descriptor->m_Flags.m_LoadInProgress)
        return false;
    if (descriptor->m_Flags.m_Loaded && descriptor->m_Ptr)
        return true;
    if (m_LoadingInProgress)
        return false;
    m_LoadingInProgress = true;

    descriptor->m_Ptr.reset();
    descriptor->m_Flags.m_Loaded = false;
    descriptor->m_Flags.m_LoadInProgress = true;

    AddLogf(Debug, "Loading scene '%s'", descriptor->m_SID.c_str());

    do {
        FileSystem::XMLFile doc;
        if (!GetFileSystem()->OpenXML(doc, descriptor->m_FID)) {
            AddLogf(Warning, "Unable to load xml for scene: '%s'", descriptor->m_SID.c_str());
            break;
        }

        auto xmlroot = doc->document_element();

        x2c::Core::Scene::SceneConfiguration_t sc;
        sc.ResetToDefault();
        if (!sc.Read(xmlroot, "Configuration")) {
            AddLogf(Error, "Failed to read configuration for scene: %s", descriptor->m_SID.c_str());
            break;
        }

        descriptor->m_Flags.m_Stateful = sc.m_Stateful;

        auto ptr = std::make_unique<ciScene>();
        auto RootEntity = m_World->GetEntityManager()->GetRootEntity();
        if (!ptr->Initialize(xmlroot, descriptor->m_SID, RootEntity, descriptor)) {
            AddLogf(Error, "Failed to initialize scene '%s'", descriptor->m_SID.c_str());
            break;
        }

        descriptor->m_Ptr.swap(ptr);
        descriptor->m_Flags.m_Loaded = true;
    } while (false);

    descriptor->m_Flags.m_LoadInProgress = false;
    m_LoadingInProgress = false;
    return descriptor->m_Flags.m_Loaded;
}

bool ScenesManager::SetNextSceneDescriptor(SceneDescriptor *descriptor) {
    m_NextSceneDescriptor = descriptor;
    //TODO
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

SceneDescriptor* ScenesManager::FindDescriptor(const std::string &SID) {
    auto sidit = m_SIDMap.find(SID);
    if (sidit != m_SIDMap.end()) {
        AddLogf(Debug, "Scene exits: sid:%s", SID.c_str());
        return sidit->second;
    }
    AddLogf(Debug, "Scene DOES NOT exits: sid:%s", SID.c_str());
    return nullptr;
}

SceneDescriptor* ScenesManager::AllocDescriptor(StarVFS::FileID fid, const std::string &SID) {
    LOCK_MUTEX(m_Lock);
    {
        auto ptr = FindDescriptor(SID);
        if (ptr) {
            AddLogf(Error, "Cannot allocate scene, SID already exists. fid:%u sid:%s", (unsigned)fid, SID.c_str());
            return nullptr;
        }
    }

    m_DescriptorTable.emplace_back();

    auto &sd = m_DescriptorTable.back();
    sd = std::make_unique<SceneDescriptor>();
    m_SIDMap[SID] = sd.get();
        
    sd->m_SID = SID;
    sd->m_FID = fid;

    sd->m_Flags.m_Valid = false;
    sd->m_Flags.m_SingleInstance = false;
    
    return sd.get();
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

} //namespace Scene::Core::MoonGlare 
