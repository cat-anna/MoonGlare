#pragma once

namespace MoonGlare::Modules {

class iModule {
public:
    virtual ~iModule() {};

    virtual int OnModuleGet(lua_State *lua) { return 0; }
    virtual void InitializeScriptApi(lua_State *lua) {}

    bool LoadSettings(const pugi::xml_node node) { return true; }
    bool SaveSettings(pugi::xml_node node) const { return true; }

    //used by scripts?
    //virtual bool PutValue(std::string key, std::string value);
    //virtual bool GetValue(std::string key, std::string &outValue) const;
};

}