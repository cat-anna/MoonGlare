//#include "luainterface.h"
#include "luasupport.h"
#include "svfslua.h"
#include "../core/nStarVFS.h"
#include <SVFSRegister.h>

using ::StarVFS::AttributeMap;

using ::StarVFS::Exporters::iExporter;
using SmartExporter = std::shared_ptr<iExporter>;

static void InstallRegister(lua_State *lua) {
    using ::StarVFS::Register;

    struct Helper {
        static int PushVectorAsTable(const std::vector<::StarVFS::String> &vec, lua_State *Lua) {
            lua_createtable(Lua, vec.size(), 0);
            int c = 0;
            for(auto &it: vec) {
                ++c;
                lua_pushinteger(Lua, c);
                lua_pushstring(Lua, it.c_str());
                lua_settable(Lua, -3);
            }
            return 1;
        }

        int GetRegisteredContainers(lua_State *Lua) {
            auto *r = (Register*)this;
            return PushVectorAsTable(r->GetRegisteredContainers(), Lua);
        }
        int GetRegisteredExporters(lua_State *Lua) {
            auto *r = (Register*)this;
            return PushVectorAsTable(r->GetRegisteredExporters(), Lua);
        }
        int GetRegisteredModules(lua_State *Lua) {
            auto *r = (Register*)this;
            return PushVectorAsTable(r->GetRegisteredModules(), Lua);
        }

        int CreateExporter(lua_State *Lua) {
            const char * exporter = lua_tostring(Lua, -1);
            if (!exporter)
                return 0;
            auto *r = (Register*)this;
            auto ptr = r->CreateExporter(exporter);
            if (!ptr)
                return 0;

            lua_createtable(Lua, 0, 0);

            lua_pushstring(Lua, "__index");
            lua_getglobal(Lua, "SubPointerCallMetaMethod");
            lua_settable(Lua, -3);
            lua_pushstring(Lua, "__parent");
            luabridge::push(Lua, ptr.get());
            lua_getmetatable(Lua, -1);
            //lua_rotate(Lua, -2, 1); //may be invalid
            lua_insert(Lua, -2);
            lua_pop(Lua, 1);
            lua_settable(Lua, -3);
            lua_pushstring(Lua, "p");
            luabridge::push(Lua, ptr.get());
            lua_settable(Lua, -3);

            lua_pushstring(Lua, "shared");
            luabridge::push(Lua, std::shared_ptr<iExporter>(ptr.release()));
            lua_settable(Lua, -3);

            lua_pushvalue(Lua, -1);
            lua_setmetatable(Lua, -2);

            return 1;
        }
    };

    luabridge::getGlobalNamespace(lua)
        .beginNamespace("api")
            .beginClass<Register>("Register")
                .addFunction("CreateModule", &Register::CreateModule)
                .addCFunction("CreateExporter", (int(Register::*)(lua_State *Lua))&Helper::CreateExporter)
                .addCFunction("GetRegisteredModules", (int(Register::*)(lua_State *Lua))&Helper::GetRegisteredModules)
                .addCFunction("GetRegisteredExporters", (int(Register::*)(lua_State *Lua))&Helper::GetRegisteredExporters)
                .addCFunction("GetRegisteredContainers", (int(Register::*)(lua_State *Lua))&Helper::GetRegisteredContainers)
            .endClass()
        .endNamespace()
        ;
}

static void InstallStarVFS(lua_State *lua) {

}

static void InstallFileHandle(lua_State *lua) {
    using ::StarVFS::FileHandle;
    struct FileHandleHelper {
        int IsDirectory() { return ((FileHandle*)this)->IsDirectory() ? 1 : 0; }
        int IsSymlink() { return ((FileHandle*)this)->IsSymlink() ? 1 : 0; }
        int IsHandleValid() { return ((FileHandle*)this)->IsHandleValid() ? 1 : 0; }

        int EnumerateChildren(lua_State *Lua) {
            auto *h = (FileHandle*)this;
            if (!h) return 0;
            int c = 0;
            lua_createtable(Lua, 0, 0);
            bool succ = h->EnumerateChildren([Lua, &c](::StarVFS::FileID fid) {
                ++c;
                lua_pushinteger(Lua, c);
                lua_pushinteger(Lua, fid);
                lua_settable(Lua, -3);
                return true;
            });
            if (!succ) {
                lua_pop(Lua, 1);
                lua_pushnil(Lua);
                return 1;
            }
            return 1;
        }
    
        int GetFileData(lua_State *Lua) {
            //no args
            auto *h = (FileHandle*)this;

            StarVFS::ByteTable ct;
            if (!h->GetFileData(ct)) {
                lua_pushnil(Lua);
                return 1;
            }

            lua_pushlstring(Lua, (char*)ct.get(), ct.byte_size());
            lua_pushinteger(Lua, ct.byte_size());
            return 2;
        }
    };

    luabridge::getGlobalNamespace(lua)
        .beginNamespace("api")
            .beginClass<FileHandle>("FileHandle")
            //.addFunction("Clone", &FileHandle::Clone)
            .addFunction("GetSize", &FileHandle::GetSize)
            //.addFunction("GetFullPath", &FileHandle::GetFullPath)
            //.addFunction("GetRWMode", &FileHandle::GetRWMode)
            .addFunction("IsDirectory", (int(FileHandle::*)())&FileHandleHelper::IsDirectory)
            .addFunction("IsSymlink", (int(FileHandle::*)())&FileHandleHelper::IsSymlink)
            .addFunction("IsValid", (int(FileHandle::*)())&FileHandleHelper::IsHandleValid)
            .addFunction("Close", &FileHandle::Close)
            .addCFunction("GetChildren", (int(FileHandle::*)(lua_State *))&FileHandleHelper::EnumerateChildren)
            .addCFunction("GetFileData", (int(FileHandle::*)(lua_State *))&FileHandleHelper::GetFileData)
            //bool GetFileData(ByteTable &data) const;
            .endClass()
        .endNamespace()
    ;
}

static void InstallAttribMap(lua_State *lua) {
    using ::StarVFS::AttributeMap;

    struct Helper {
        int GetAttribs(lua_State *l) {
            auto *a = (AttributeMap*)this;
            auto v = a->GetAttributeNames();

            lua_createtable(l, v.size(), 0);

            for (size_t i = 0; i < v.size(); ++i) {
                lua_pushinteger(l, i);
                lua_pushstring(l, v[i].c_str());
                lua_settable(l, -3);
            }

            return 1;
        }
        int Set(const char *name, const char *value) {
            if (!name || !value)
                return 0;
            auto *a = (AttributeMap*)this;

            return a->SetAttribute(name, value) ? 1 : 0;
        }
        int Get(lua_State *l) {
            const char *name = lua_tostring(l, -1);
            if (!name) {
                lua_pushnil(l);
                return 1;
            }

            auto *a = (AttributeMap*)this;
            StarVFS::String v;
            if (!a->SetAttribute(name, v)) {
                lua_pushnil(l);
                return 1;
            }
            lua_pushstring(l, v.c_str());
            return 1;
        }
    };

    luabridge::getGlobalNamespace(lua)
        .beginNamespace("api")
            .beginClass<AttributeMap>("AttributeMap")
                .addCFunction("GetAttributeList", (int(AttributeMap::*)(lua_State *))&Helper::GetAttribs)
                .addCFunction("GetAttribute", (int(AttributeMap::*)(lua_State *))&Helper::Get)
                .addFunction("SetAttribute", (int(AttributeMap::*)(const char *, const char *))&Helper::Set)
            .endClass()
        .endNamespace()
        ;
}

static void InstalliContainer(lua_State *lua) {
    using ::StarVFS::AttributeMap;
    using ::StarVFS::Containers::iContainer;
    using ::StarVFS::Containers::VirtualFileContainer;
    luabridge::getGlobalNamespace(lua)
        .beginNamespace("api")
            .deriveClass<iContainer, AttributeMap>("iContainer")
            .endClass()
            .deriveClass<VirtualFileContainer, iContainer>("VirtualFileContainer")
                .addFunction("InjectFile", &VirtualFileContainer::InjectFile)
            .endClass()
        .endNamespace()
        ;
    //.beginClass<::StarVFS::iContainer>("iContainer")
    //.endClass()
}

static void InstalliModule(lua_State *lua) {
    using ::StarVFS::AttributeMap;
    using ::StarVFS::Modules::iModule;
    luabridge::getGlobalNamespace(lua)
        .beginNamespace("api")
            .deriveClass<iModule, AttributeMap>("iModule")
                .addFunction("Enable", &iModule::Enable)
                .addFunction("Disable", &iModule::Disable)
            .endClass()
        .endNamespace()
        ;
}

static void InstalliExporter(lua_State *lua) {
    struct Helper {
        int DoExport(const char *vfsbase, const char *localfile) {
            auto This = (iExporter*)this;
            return (int)This->DoExport(vfsbase, localfile);
        }
    };;
    luabridge::getGlobalNamespace(lua)
        .beginNamespace("api")
            .deriveClass<iExporter, AttributeMap>("iExporter")
                .addFunction("DoExport", (int(iExporter::*)(const char*, const char *))&Helper::DoExport)
            .endClass()
            .beginClass<SmartExporter>("SmartExporter")
            .endClass()
        .endNamespace()
        ;
}

//-------------------------------------------------------------------------------------------------

void svfslua::Install(lua_State *lua) {
    try {
        InstallRegister(lua);
        InstallStarVFS(lua);
        InstallFileHandle(lua);

        InstallAttribMap(lua);
        InstalliContainer(lua);
        InstalliModule(lua);
        InstalliExporter(lua);
    }
    catch (...) {
        printf("Failed to install svfs lua interface!\n");
        exit(1);
    }
}
