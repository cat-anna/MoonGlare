#include <pch.h>
#include <nfMoonGlare.h>
#include <Engine/Modules/ModuleManager.h>
#include <Engine/Core/Engine.h>
#include <Engine/Core/DataManager.h>
#include <Engine/Core/Configuration.Runtime.h>
#include <Engine/Core/Scene/ScenesManager.h>
#include <Engine/Font.h>
#include <Foundation/Scripts/iLuaRequire.h>
#include <Core/Scripts/LuaApi.h>
#include <Core/Scripts/ScriptEngine.h>

#include <StarVFS/core/nStarVFS.h>

namespace MoonGlare {
namespace Core {
namespace Data {

RegisterApiBaseClass(Manager, &Manager::RegisterScriptApi);

Manager::Manager(World *world) : world(world) {
    ASSERT(world);

    OrbitLogger::LogCollector::SetChannelName(OrbitLogger::LogChannels::Resources, "RES");
}

Manager::~Manager() {
    m_Fonts.clear();
}

//-------------------------------------------------------------------------------------------------

void Manager::RegisterScriptApi(::ApiInitializer &api) {
    api
        .beginClass<RuntimeConfiguration>("cRuntimeConfiguration")
            .addData("scene", &RuntimeConfiguration::scene)
            .addData("consoleFont", &RuntimeConfiguration::consoleFont)
        .endClass()                                
    ;
}

//------------------------------------------------------------------------------------------

bool Manager::InitModule(StarVFS::Containers::iContainer *Container) {
    MoonGlareAssert(Container);

    AddLogf(Hint, "Loaded container '%s'", Container->GetContainerURI().c_str());

    LoadInitScript(Container);

    return true;
}

void Manager::LoadInitScript(StarVFS::Containers::iContainer *Container) {
    MoonGlareAssert(Container);

    struct RTCfg : public Scripts::iRequireRequest {
        RTCfg(RuntimeConfiguration*r):currconf(*r), rtconf(r){}
        RuntimeConfiguration currconf;
        RuntimeConfiguration *rtconf;
        bool requested = false;
        bool OnRequire(lua_State *lua, std::string_view name) override {
            luabridge::push(lua, &currconf);
            requested = true;
            return true;
        }
        ~RTCfg() {
            if (requested)
                *rtconf = currconf;
        }
    };

    auto cfid = Container->FindFile("/init.lua");
    StarVFS::ByteTable data;
    if (!Container->GetFileData(cfid, data)) {
        //FIXME: read error / file does exists -> is it detectable
        AddLogf(Error, "Failed to read init script! (cid:%d;cfid:%d)", (int)Container->GetContainerID(), (int)cfid);
    }
    else {
        RTCfg require(world->GetRuntimeConfiguration());
        auto rmod = world->GetScriptEngine()->QuerryModule<Scripts::iRequireModule>();
        rmod->RegisterRequire("RuntimeConfiguration", &require);
        
        try {
            std::string furi = fmt::format("cfid://{}/{}", (int)Container->GetContainerID(), "init.lua");
            world->GetScriptEngine()->ExecuteCode((char*)data.get(), data.byte_size(), furi.c_str());
        }
        catch (const std::exception &e) {
            AddLogf(Error, "Init script execution error (cid:%d;cfid:%d) : %s", (int)Container->GetContainerID(), (int)cfid, e.what());
        }
        catch (...) {
            AddLogf(Error, "Init script execution error (cid:%d;cfid:%d) : unknown error", (int)Container->GetContainerID(), (int)cfid);
        }

        rmod->RegisterRequire("RuntimeConfiguration", nullptr);
    }
}

//------------------------------------------------------------------------------------------

void Manager::InitFonts() {
    iFileSystem *fs = nullptr;
    world->GetObject(fs);

    StarVFS::DynamicFIDTable fontfids;
    fs->FindFilesByExt(".ttf", fontfids);

    for (auto fileid : fontfids) {
        std::string name = fs->GetFileName(fileid);

        while (name.back() != '.')
            name.pop_back();
        name.pop_back();

        auto uri = "file://" + fs->GetFullFileName(fileid);
        AddLogf(Debug, "Found font: %s -> %s", uri.c_str(), name.c_str());

        m_Fonts[name] = std::make_shared<DataClasses::Font>(uri);
    }
}   

DataClasses::FontPtr Manager::GetFont(const string &Name) {
    auto it = m_Fonts.find(Name);
    DataClasses::FontPtr ptr = nullptr;
    if (it == m_Fonts.end()) {
        AddLogf(Error, "Font '%s' not found", Name.c_str());
        return nullptr;
    } else {
        ptr = it->second;
        if (!ptr->IsReady()) {
            if (!ptr->Initialize()) {
                AddLogf(Error, "Unable to initialize font '%s'", Name.c_str());
                return nullptr;
            }
        }
        return ptr;
    }
}

} // namespace Data
} // namespace Core
} // namespace MoonGlare
