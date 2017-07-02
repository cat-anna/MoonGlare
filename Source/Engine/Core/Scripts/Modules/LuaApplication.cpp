#include <pch.h>
#include <nfMoonGlare.h>
#include <iConsole.h>

#include "../ScriptEngine.h"
#include "StaticModules.h"

#include <iApplication.h>

namespace MoonGlare::Core::Scripts::Modules {

template<typename T, void(T::*F)()>
int ClosureObjectCall(lua_State* lua) {
    T *t = reinterpret_cast<T*>(lua_touserdata(lua, lua_upvalueindex(1)));
    (t->*F)();
    return 0;
}


void StaticModules::InitApplication(lua_State *lua, World *world) {
    DebugLogf(Debug, "Initializing Application module");
    
    using Application::iApplication;
    void *app = GetApplication();

    luabridge::getGlobalNamespace(lua)
        .beginNamespace("Application")
            .addCClosure("Exit", &ClosureObjectCall<iApplication, &iApplication::Exit>, app)
            .addCClosure("Restart", &ClosureObjectCall<iApplication, &iApplication::Restart>, app)
#ifdef DEBUG_SCRIPTAPI
            //.addCClosure("Abort", &ClosureObjectCall<iApplication, &iApplication::Abort>, app)
#endif
        .endNamespace()
        ;
}

} //namespace MoonGlare::Core::Scripts::Modules
