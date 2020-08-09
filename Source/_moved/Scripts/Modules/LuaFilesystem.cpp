#include <Foundation/Scripts/LuaStackOverflowAssert.h>
#include <interface_map.h>

#include "../ApiInit.h"
#include "../ErrorReporting.h"
#include "../iDynamicModule.h"
#include "../iLuaRequire.h"

#include "LuaFilesystem.h"

namespace MoonGlare::Scripts::Modules {

LuaFileSystemModule::LuaFileSystemModule(lua_State *lua, InterfaceMap *world) {
    world->GetInterface<iRequireModule>()->RegisterRequire("FileSystem", this);
    fs = world->GetInterface<iFileSystem>();
}

LuaFileSystemModule::~LuaFileSystemModule() {}

/*@ [RequireModules/LuaFileSystemModule] FileSystem module
    This module allows to access internal file system
@*/

ApiInitializer LuaFileSystemModule::RegisterScriptApi(ApiInitializer api) {
    return api.beginClass<LuaFileSystemModule>("LuaFileSystemModule")
        .addCFunction("ReadFileContent", &LuaFileSystemModule::ReadFileContent)
        .addCFunction("ReadJSON", &LuaFileSystemModule::ReadJSON)
        .addCFunction("Enumerate", &LuaFileSystemModule::EnumerateFolder)
        .addCFunction("FindFilesByExt", &LuaFileSystemModule::FindFilesByExt)
        .endClass();
}

bool LuaFileSystemModule::OnRequire(lua_State *lua, std::string_view name) {
    luabridge::push<LuaFileSystemModule *>(lua, this);
    return true;
}

//-------------------------------------------------------------------------------------------------

/*@ [LuaFileSystemModule/_] `FileSystem:ReadFileContent(fileURI)`
    Read entire content of requested file. In file does not exist nil is
returned.
@*/
int LuaFileSystemModule::ReadFileContent(lua_State *lua) {
    if (!lua_isstring(lua, -1)) {
        AddLogf(ScriptRuntime,
                "Invalid argument #1 to FileSystem::ReadFileContent");
        return 0;
    }

    std::string uri = lua_tostring(lua, -1);

    StarVFS::ByteTable bt;
    if (!fs->OpenFile(bt, uri)) {
        AddLogf(ScriptRuntime,
                "FileSystem::ReadFileContent : Cannot open file %s",
                uri.c_str());
        return 0;
    }

    lua_pushlstring(lua, (const char *)bt.get(), bt.byte_size());
    return 1;
}

/*@ [LuaFileSystemModule/_] `FileSystem:ReadJSON(fileURI)`
    Read json file. On success parsed file content is returned, nil on error.
@*/
int LuaFileSystemModule::ReadJSON(lua_State *lua) {
    if (ReadFileContent(lua) != 1) {
        AddLogf(ScriptRuntime,
                "FileSystem::ReadJSON : Cannot Read file content");
        return 0;
    }

    // uri fileData

    lua_getglobal(lua, "require"); // uri fileData requireF
    if (lua_isnil(lua, -1)) {
        AddLogf(ScriptRuntime, "FileSystem::ReadJSON : Internal error");
        return 0;
    }
    lua_pushstring(lua, "json"); // uri fileData requireF 'json'
    lua_call(lua, 1, 1);         // uri fileData jsonLib
    if (lua_isnil(lua, -1)) {
        AddLogf(ScriptRuntime, "FileSystem::ReadJSON : Cannot open json lib");
        return 0;
    }
    lua_getfield(lua, -1, "decode"); // uri fileData jsonLib decodeFunc
    if (lua_isnil(lua, -1)) {
        AddLogf(ScriptRuntime,
                "FileSystem::ReadJSON : Cannot find json.decode method");
        return 0;
    }
    lua_insert(lua, -3); // uri fileData jsonLib
    lua_pop(lua, 1);     // uri decodeFunc fileData
    lua_call(lua, 1, 1); // uri jsonObject
    if (lua_isnil(lua, -1)) {
        AddLogf(ScriptRuntime, "FileSystem::ReadJSON : JSON parse failure");
        return 0;
    }

    return 1;
}

/*@ [LuaFileSystemModule/_] `FileSystem:EnumerateFolder(folderURI)`
    Function enumerates requested directory and return table with found entries.
    Nil value is returned on error.
    Each element of returned table has following fields:
    * Name
    * RelativePath
    * FileId
    * IsFolder
@*/
int LuaFileSystemModule::EnumerateFolder(lua_State *lua) {
    // top=1 -> self
    if (!lua_isstring(lua, 2)) {
        AddLogf(ScriptRuntime,
                "Invalid argument #1 to FileSystem::EnumerateFolder");
        return 0;
    }

    MoonGlare::Scripts::LuaStackOverflowAssert overflow(lua);

    std::string path = lua_tostring(lua, 2);

    FileInfoTable fit;
    if (!fs->EnumerateFolder(path, fit, lua_toboolean(lua, 3))) {
        AddLogf(ScriptRuntime,
                "FileSystem::EnumerateFolder : Enumeration failed");
        return overflow.ReturnArgs(0);
    }

    lua_createtable(lua, 0, 0); // baseTable

    int index = 1;
    for (auto &item : fit) {
        lua_createtable(lua, 0, 0); // baseTable item

        lua_pushstring(lua, item.m_RelativeFileName.c_str());
        lua_setfield(lua, -2, "RelativePath");

        lua_pushstring(lua, item.m_FileName.c_str());
        lua_setfield(lua, -2, "Name");

        lua_pushinteger(lua, item.m_FID);
        lua_setfield(lua, -2, "FileId");

        lua_pushboolean(lua, item.m_IsFolder);
        lua_setfield(lua, -2, "IsFolder");

        lua_pushinteger(lua, index); // baseTable item Index
        ++index;

        lua_insert(lua, -2); // baseTable Index item

        lua_settable(lua, -3); // baseTable
    }

    return overflow.ReturnArgs(1);
}

/*@ [LuaFileSystemModule/_] `FileSystem:FindFilesByExt(ext)`
    Function enumerates entire filesystem and looks for files with specified
extension. Nil value is returned on error, on success table with found filepaths
is returned
    TODO: with dot or without?
@*/
int LuaFileSystemModule::FindFilesByExt(lua_State *lua) {
    static constexpr char *ScriptFunctionName = "FileSystem::FindFilesByExt";

    const char *ext = lua_tostring(lua, -1);
    if (!ext) {
        LuaReportInvalidArg(lua, 1, string);
        return 0;
    }

    StarVFS::DynamicFIDTable fidT;
    fs->FindFilesByExt(ext, fidT);

    lua_createtable(lua, static_cast<int>(fidT.size()), 0);

    int index = 1;
    for (auto fid : fidT) {
        std::string fname = fs->GetFullFileName(fid);

        lua_pushinteger(lua, index);
        lua_pushstring(lua, fname.c_str());
        lua_settable(lua, -2);

        ++index;
    }

    return 1;
}

} // namespace MoonGlare::Scripts::Modules
