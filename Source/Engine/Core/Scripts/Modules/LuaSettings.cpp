#include <pch.h>
#include <nfMoonGlare.h>

#include "../../Engine.h"
#include <Application.h>
#include "../ScriptEngine.h"

#include "LuaSettings.h"

#include <Foundation/Scripts/ErrorHandling.h>
#include <Foundation/Settings.h>

namespace MoonGlare::Core::Scripts::Modules {
using namespace MoonGlare::Scripts;

struct LuaSettingsModule::SettingsObject {
    LuaSettingsModule *owner;
    Application *application;
    std::shared_ptr<MoonGlare::Settings> settings;

    ~SettingsObject() {
        Cancel();
    }

    void Apply() {
        if (settingsChangedMap.empty()) {
            return;
        }

        for (auto &i : settingsChangedMap) {
            try {
                i.second.settingInfo->provider->Set(i.second.prefix, i.second.id, i.second.value);
            }
            catch (Settings::iSettingsProvider::InvalidSettingId) {
                AddLog(Error, fmt::format("Apply failed(InvalidSettingId): {}.{} = {}", i.second.prefix.data(), i.second.id.data(), ValueVariantToString(i.second.value)));
                Core::GetEngine()->Exit();
                throw LuaPanic(fmt::format("Invalid setting '{}'", i.second.id.data()));
            }
            catch (const std::bad_variant_access &eacces) {
                AddLog(Error, fmt::format("Apply failed(bad_variant_access): {}.{} = {}", i.second.prefix.data(), i.second.id.data(), ValueVariantToString(i.second.value)));
                Core::GetEngine()->Exit();
                throw LuaPanic(fmt::format("Invalid setting value type '{}' -> '{}'", i.second.id.data(), eacces.what()));
            }
        }

        application->SettingsChanged();

        if (needRestart) {
            application->SetRestart(true);
            Core::GetEngine()->Exit();
        }
    }
    void Cancel() {
    }

    int lua_Get(lua_State* lua) {
        std::string_view rawid = luaL_checkstring(lua, -1);

        {
            auto cacheit = settingsChangedMap.find(rawid.data());
            if (cacheit != settingsChangedMap.end()) {
                return PushValueVariant(lua, cacheit->second.value);
            }
        }

        std::string_view provider, id;
        auto *s = FindSetting(rawid, provider, id);
        if (!s) {
            if (settings->HasValue(rawid.data())) {
                return PushValueVariant(lua, settings->GetValue(rawid.data()));
            }
            throw LuaPanic(fmt::format("Cannot find setting {}", id.data()));
        }

        try {
            auto vv = s->provider->Get(provider, id);
            return PushValueVariant(lua, vv);
        }
        catch (Settings::iSettingsProvider::InvalidSettingId) {
            throw LuaPanic(fmt::format("Invalid setting '{}'", rawid.data()));
        }
    }
    int lua_Set(lua_State* lua) {
        std::string_view rawid = luaL_checkstring(lua, -2);

        std::string_view provider, id;
        Settings::ApplyMethod am;
        auto vv = GetValueVariant(lua, -1);
        auto *s = FindSetting(rawid, provider, id);
        if (!s) {
            if (settings->HasValue(rawid.data())) {
                am = settings->SetValue(rawid.data(), vv);
                needRestart = needRestart || am == Settings::ApplyMethod::Restart;
            } 
            return 0;
        }

        if (s->settingData.applyMethod == Settings::ApplyMethod::Immediate) {
            try {
                s->provider->Set(provider, id, vv);
                application->SettingsChanged();
            }
            catch (Settings::iSettingsProvider::InvalidSettingId) {
                __debugbreak();
                throw LuaPanic(fmt::format("Invalid setting '{}'", rawid.data()));
            }
            catch (const std::bad_variant_access &eacces) {
                __debugbreak();
                throw LuaPanic(fmt::format("Invalid setting value type '{}' -> '{}'", rawid.data(), eacces.what()));
            }
        }
        else {
            settingsChangedMap[std::string(rawid)] = ChangedSettingInfo{ s, std::string(provider), std::string(id), vv };
            needRestart = true;
        }
        return 0;
    }
    int lua_GetDefault(lua_State* lua) {
        //TODO
        return 0;
    }
    int lua_ListAll(lua_State* lua) {
        lua_newtable(lua);

        int cnt = 1;
        for (auto &provider : owner->providerMap) {
            for (auto &setting : provider.second->GetSettingList(provider.first)) {
                auto fullid = fmt::format("{}.{}", provider.first, setting.first);
                lua_pushinteger(lua, cnt);
                lua_pushstring(lua, fullid.c_str());
                lua_settable(lua, -3);
                ++cnt;
            }
        }

        for (auto &item : settings->GetKeys()) {
            lua_pushinteger(lua, cnt);
            lua_pushstring(lua, item.c_str());
            lua_settable(lua, -3);
            ++cnt;
        }

        return 1;
    }
    void Dump() {
        AddLog(Info, "Registered settings:");
        for (auto &provider : owner->providerMap) {
            for (auto &setting : provider.second->GetSettingList(provider.first)) {
                auto vv = provider.second->Get(provider.first, setting.first);
                AddLog(Info, fmt::format("{}.{} = {}", provider.first, setting.first, ValueVariantToString(vv)));
            }
        }
        if (!settingsChangedMap.empty()) {
            AddLog(Info, "Pending changes:");
            for (auto &i : settingsChangedMap) {
                AddLog(Info, fmt::format("{} = {}", i.first, ValueVariantToString(i.second.value)));
            }
        }
    }

    struct SettingInfo {
        Settings::iSettingsProvider *provider;
        Settings::Setting settingData;
    };
    std::unordered_map<std::string, SettingInfo> settingCacheMap;

    struct ChangedSettingInfo {
        const SettingInfo *settingInfo;
        std::string prefix;
        std::string id;
        Settings::ValueVariant value;
    };
    std::unordered_map<std::string, ChangedSettingInfo> settingsChangedMap;
    std::unordered_map<std::string, ChangedSettingInfo> settingsAppliedMap;
    bool needRestart = false;

    const SettingInfo* FindSetting(std::string_view rawid, std::string_view &provider, std::string_view &id) {
        auto dot = rawid.find('.');
        provider = rawid.substr(0, dot);
        id = rawid;
        id.remove_prefix(dot + 1);

        auto srawid = std::string(rawid);

        auto cacheit = settingCacheMap.find(srawid);
        if (cacheit != settingCacheMap.end()) {
            return &cacheit->second;
        }

        auto pit = owner->providerMap.find(std::string(provider));
        if (pit == owner->providerMap.end()) {
            return nullptr;
        }

        try {
            auto data = pit->second->GetSettingList(provider)[id.data()];

            auto &item = settingCacheMap[srawid];
            item.provider = pit->second;
            item.settingData = data;
            return &item;
        }
        catch (...) {
            return nullptr;
        }
    }

    static int PushValueVariant(lua_State *lua, Settings::ValueVariant v) {
        return std::visit([lua](auto &value) -> int {
            if constexpr (std::is_same_v<nullptr_t, std::remove_reference_t<decltype(value)>>) {
                lua_pushnil(lua);
            } else {
                luabridge::push(lua, value);
            }
            return 1;
        }, v);
    }

    template<typename T> static std::string tostr(T t) { return std::to_string(t); }
    static std::string tostr(nullptr_t) { return "[NULL]"; }
    static std::string tostr(const std::string &str) { return fmt::format("\"{}\"", str); }
    static std::string ValueVariantToString(Settings::ValueVariant v) {
        return std::visit([](auto &value) -> std::string {
            return tostr(value);
        }, v);
    }

    static Settings::ValueVariant GetValueVariant(lua_State *lua, int idx, bool preferInteger = false) {
        switch (lua_type(lua, idx)) {
        case LUA_TBOOLEAN:
            return lua_toboolean(lua, idx) != 0;
        case LUA_TNUMBER:
            if (preferInteger) {
                return static_cast<int>(lua_tonumber(lua, idx));
            }
            else {
                return static_cast<float>(lua_tonumber(lua, idx));
            }
        case LUA_TSTRING:
            return std::string(lua_tostring(lua, idx));
        case LUA_TNIL:
        default:
            return nullptr;
        }
    }
};

//-------------------------------------------------------------------------------------------------

LuaSettingsModule::LuaSettingsModule(lua_State *lua, World *world) : world(world) {
    world->GetScriptEngine()->QuerryModule<iRequireModule>()->RegisterRequire("Settings", this);
}

LuaSettingsModule::~LuaSettingsModule() { }

//-------------------------------------------------------------------------------------------------

void LuaSettingsModule::RegisterProvider(std::string prefix, Settings::iSettingsProvider *provider) {
    //FIXME: make reregistration impossible
    providerMap[prefix] = provider;
}

bool LuaSettingsModule::OnRequire(lua_State *lua, std::string_view name) {
    if (!scriptApiRegistered)
        RegisterScriptApi(lua);

    luabridge::push(lua, SettingsObject{ this, world->GetInterface<Application>(), world->GetSharedInterface<MoonGlare::Settings>() });

    return true;
}

//-------------------------------------------------------------------------------------------------

void LuaSettingsModule::RegisterScriptApi(lua_State *lua) {
    scriptApiRegistered = true;

    luabridge::getGlobalNamespace(lua)
        .beginNamespace("api")
            .beginClass<SettingsObject>("SettingsHandler")
                .addCFunction("Get", &SettingsObject::lua_Get)
                .addCFunction("Set", &SettingsObject::lua_Set)
                .addCFunction("GetDefault", &SettingsObject::lua_GetDefault)
                .addCFunction("ListAll", &SettingsObject::lua_ListAll)

                .addFunction("Apply", &SettingsObject::Apply)
                //.addFunction("Cancel", &SettingsObject::Cancel)
#ifdef DEBUG_SCRIPTAPI
                .addFunction("Dump", &SettingsObject::Dump)
#endif
           .endClass()
        .endNamespace()
        ;
}

} //namespace MoonGlare::Core::Scripts::Modules
                             