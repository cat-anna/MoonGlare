#include <pch.h>
#include <nfMoonGlare.h>

#include "StaticModules.h"
#include <Application.h>

namespace MoonGlare::Core::Scripts::Modules {

template<typename T, void(T::*F)()>
static int ClosureObjectCall(lua_State* lua) {
    T *t = reinterpret_cast<T*>(lua_touserdata(lua, lua_upvalueindex(1)));
    (t->*F)();
    return 0;
}

static const char *GetVersionString() { return VersionString; }
static const char *GetCompilationDate() { return CompilationDate; }
static const char *GetApplicationName() { return ApplicationName; }

//TODO: Make Application module a dynamic one

/*@ [StaticModules/LuaApplicationModule] Application module
    Provide acces to some info about the engine and perform application restart or exit.  
    Accessible through global `Application`
@*/
void StaticModules::InitApplication(lua_State *lua, World *world) {
    DebugLogf(Debug, "Initializing Application module");
    
    void *app = world->GetInterface<Application>();

    luabridge::getGlobalNamespace(lua)
        .beginNamespace("Application")
/*@ [LuaApplicationModule/_] `Application:Exit()`
    Perform gracefull engine shutdown @*/
            .addCClosure("Exit", &ClosureObjectCall<Application, &Application::Exit>, app)
/*@ [LuaApplicationModule/_] `Application:Restart()`
    Perform gracefull engine restart @*/            
            .addCClosure("Restart", &ClosureObjectCall<Application, &Application::Restart>, app)

/*@ [LuaApplicationModule/_] `Application.versionString`
    TODO @*/   
            .addProperty("versionString", GetVersionString)
/*@ [LuaApplicationModule/_] `Application.compilationDate`
    TODO @*/            
            .addProperty("compilationDate", GetCompilationDate)
/*@ [LuaApplicationModule/_] `Application.name`
    TODO @*/            
            .addProperty("name", GetApplicationName)

#ifdef DEBUG_SCRIPTAPI
            //.addCClosure("Abort", &ClosureObjectCall<iApplication, &iApplication::Abort>, app)
#endif
        .endNamespace()
        ;
}

} //namespace MoonGlare::Core::Scripts::Modules
