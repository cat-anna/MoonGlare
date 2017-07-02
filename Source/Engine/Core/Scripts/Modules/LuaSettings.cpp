#include <pch.h>
#include <nfMoonGlare.h>

#include "../ScriptEngine.h"

#include "LuaSettings.h"

namespace MoonGlare::Core::Scripts::Modules {

struct LuaSettingsModule::SettingsObject {
    LuaSettingsModule *owner;

    ~SettingsObject() {
    }

    void Apply() {}
    void Cancel() {}

    int lua_Get(lua_State* lua) {
        std::string_view rawid = luaL_checkstring(lua, -1);

        {
            auto cacheit = settingsChangedMap.find(rawid.data());
            if (cacheit != settingsChangedMap.end()) {
                return PushValueVariant(lua, cacheit->second.value);
            }
        }

        std::string_view id;
        auto *s = FindSetting(rawid, id);
        if (!s)
            throw eLuaPanic(fmt::format("Cannot find setting {}", id.data()));

        try {
            auto vv = s->provider->Get(id.data());
            return PushValueVariant(lua, vv);
        }
        catch (Settings::iSettingsProvider::InvalidSettingId) {
            throw eLuaPanic(fmt::format("Invalid setting '{}'", rawid.data()));
        }
    }
    int lua_Set(lua_State* lua) {
        std::string_view rawid = luaL_checkstring(lua, -2);

        std::string_view id;
        auto *s = FindSetting(rawid, id);

        auto vv = GetValueVariant(lua, -1);
        if (s->settingData.immediateApply) {
            try {
                s->provider->Set(id.data(), vv);
            }
            catch (Settings::iSettingsProvider::InvalidSettingId) {
                throw eLuaPanic(fmt::format("Invalid setting '{}'", rawid.data()));
            }
            catch (const std::bad_variant_access &eacces) {
                throw eLuaPanic(fmt::format("Invalid setting value type '{}' -> '{}'", rawid.data(), eacces.what()));
            }
        }
        else {
            settingsChangedMap[std::string(rawid)] = ChangedSettingInfo{ s, std::string(rawid), vv };
        }
        return 0;
    }
    int lua_GetDefault(lua_State* lua) {
        return 0;
    }

    int lua_ListAll(lua_State* lua) {
        lua_newtable(lua);

        int cnt = 1;
        for (auto &provider : owner->providerMap) {
            for (auto &setting : provider.second->GetSettingList()) {
                auto fullid = fmt::format("{}.{}", provider.first, setting.first);
                lua_pushinteger(lua, cnt);
                lua_pushstring(lua, fullid.c_str());
                lua_settable(lua, -3);
                ++cnt;
            }
        }

        return 1;
    }

    struct SettingInfo {
        Settings::iSettingsProvider *provider;
        Settings::Setting settingData;
    };
    std::unordered_map<std::string, SettingInfo> settingCacheMap;

    struct ChangedSettingInfo {
        const SettingInfo *settingInfo;
        std::string id;
        Settings::ValueVariant value;
    };
    std::unordered_map<std::string, ChangedSettingInfo> settingsChangedMap;

    const SettingInfo* FindSetting(std::string_view rawid, std::string_view &id) {
        auto dot = rawid.find('.');
        auto provider = rawid.substr(0, dot);
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

        auto &item = settingCacheMap[srawid];
        item.provider = pit->second;
        item.settingData = item.provider->GetSettingList()[std::string(id)];

        return &item;
    }

    static int PushValueVariant(lua_State *lua, Settings::ValueVariant v) {
        return std::visit([lua](auto &value) -> int {
            if (std::is_same_v<nullptr_t, std::remove_reference_t<decltype(value)>>) {
                lua_pushnil(lua);
            }
            else {
                luabridge::push(lua, value);
            }
            return 1;
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
//-------------------------------------------------------------------------------------------------

LuaSettingsModule::LuaSettingsModule(lua_State *lua, World *world) {
    world->GetScriptEngine()->QuerryModule<iRequireModule>()->RegisterRequire("Settings", this);
}

LuaSettingsModule::~LuaSettingsModule() {}

//-------------------------------------------------------------------------------------------------

void LuaSettingsModule::RegisterProvider(std::string prefix, Settings::iSettingsProvider *provider) {
    //FIXME: make reregistration impossible
    providerMap[prefix] = provider;
}

bool LuaSettingsModule::OnRequire(lua_State *lua, std::string_view name) {
    if (!scriptApiRegistered)
        RegisterScriptApi(lua);

    luabridge::push(lua, SettingsObject{ this });

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

        //.addFunction("Apply", &SettingsObject::Apply)
        //.addFunction("Cancel", &SettingsObject::Cancel)
        .endClass()
        .endNamespace()
        ;
}


} //namespace MoonGlare::Core::Scripts::Modules
