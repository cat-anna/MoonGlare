#include <lua.hpp>
#include <Foundation/Scripts/LuaBridge/LuaBridge.h>

#include "ScriptApi.h"
#include "Renderer.h"
#include "RenderDevice.h"
#include "Device/GLFWContext.h"
#include "Resources/ResourceManager.h"     

#include <Renderer/Resources/Shader/ShaderResource.h>
#include <Renderer/Resources/MaterialManager.h>

namespace MoonGlare::Renderer {

struct MaterialApi {
    void ReloadMaterials() {
        rendererFacade->GetResourceManager()->GetMaterialManager().ReloadMaterials();
    }

    RendererFacade* rendererFacade = nullptr;
};

//---------------------------------------------------------------------------------------

struct ShaderApi {
    void ReloadShader(const char *name) {
        assert(this);
        DebugLogf(Warning, "Reloading shader %s", name);
        m_RendererFacade->GetResourceManager()->GetShaderResource().Reload(name);
    }
    void ReloadAllShaders() {
        assert(this);
        DebugLogf(Warning, "Reloading all shaders");
        m_RendererFacade->GetResourceManager()->GetShaderResource().ReloadAll();
    }
    void DumpShaders() {
        assert(this);
        DebugLogf(Warning, "Dumping all shaders");
        std::ostringstream ss;
        Space::OFmtStream fss(ss);
        m_RendererFacade->GetResourceManager()->GetShaderResource().Dump(fss);
        DebugLog(Info, "Loaded shaders: \n" << ss.str());
    }

    RendererFacade* m_RendererFacade = nullptr;
};

//---------------------------------------------------------------------------------------

struct TextureApi {
    RendererFacade* m_RendererFacade = nullptr;
};

//---------------------------------------------------------------------------------------

struct ContextApi {
    void Install(lua_State *lua) {
        luabridge::getGlobalNamespace(lua)
        //.beginNamespace("api")
        //.endNamespace()
        .beginNamespace("Renderer")
            .addObjectFunction("GetMonitorCount", this, &ContextApi::GetMonitorCount)
            .addObjectCFunction("GetMonitorModes", this, &ContextApi::GetMonitorModes)
            .addObjectCFunction("GetMonitorCurrentMode", this, &ContextApi::GetMonitorCurrentMode)
            .addObjectFunction("CaptureScreenShot", this, &ContextApi::CaptureScreenShot)
        .endNamespace()
        ;
    }

    void CaptureScreenShot() {
       //m_RendererFacade->GetContextImpl()->CaptureScreenShot();
    }
    int GetMonitorCount() {
        return Device::GLFWContext::GetMonitorCount();
    }
    int GetMonitorModes(lua_State *lua) {
        int monitorid = lua_tointeger(lua, -1);

        int c;
        auto mont = glfwGetMonitors(&c);

        if (monitorid < 0 || monitorid > c)
            return 0;

        auto modes = Device::GLFWContext::GetMonitorModes(monitorid);
        const GLFWvidmode* currmode = glfwGetVideoMode(mont[monitorid]);

        lua_createtable(lua, 0, 0);
        
        int index = 0;
        for (const auto &mode : modes) {
            ++index;

            lua_pushinteger(lua, index);
            lua_createtable(lua, 0, 0);

            lua_pushinteger(lua, mode.height);
            lua_setfield(lua, -2, "Height");

            lua_pushinteger(lua, mode.width);
            lua_setfield(lua, -2, "Width");

            if (currmode->height == mode.height && currmode->width == mode.width) {
                lua_pushboolean(lua, 1);
                lua_setfield(lua, -2, "Current");
            }

            lua_settable(lua, -3);
        }

        return 1;
    }
    int GetMonitorCurrentMode(lua_State *lua) {
        int monitorid = lua_tointeger(lua, -1);

        int c;
        auto mont = glfwGetMonitors(&c);

        if (monitorid < 0 || monitorid > c)
            return 0;

        const GLFWvidmode* currmode = glfwGetVideoMode(mont[monitorid]);

        lua_createtable(lua, 0, 0);

        lua_pushinteger(lua, currmode->height);
        lua_setfield(lua, -2, "Height");

        lua_pushinteger(lua, currmode->width);
        lua_setfield(lua, -2, "Width");

        return 1;
    }
    RendererFacade* m_RendererFacade = nullptr;
};

//---------------------------------------------------------------------------------------

struct HandleApi {
    void Install(lua_State *lua) {
        luabridge::getGlobalNamespace(lua)
            .beginNamespace("api")
                .beginClass<MeshResourceHandle>("MeshResourceHandle")
                .endClass()
            .endNamespace()
            ;
    }

    RendererFacade* m_RendererFacade = nullptr;
};

//---------------------------------------------------------------------------------------

struct ScriptApi::ScriptApiImpl {
    ScriptApiImpl(RendererFacade * facade) : 
            m_RendererFacade(facade),
            m_ShaderApi{ facade }, 
            m_TextureApi{ facade },
            m_ContextApi{ facade },
            materialApi{ facade },
            hadleApi{ facade }
    {
        assert(m_RendererFacade);
    }

    void Install(lua_State *lua) {
        assert(lua);

        m_ContextApi.Install(lua);
        hadleApi.Install(lua);
#ifdef DEBUG_SCRIPTAPI
        InstallDebug(lua);
#endif
    }

    void InstallDebug(lua_State *lua) {
        luabridge::getGlobalNamespace(lua)
        .beginNamespace("Debug")
            .beginNamespace("Renderer")
                .addObjectFunction("ReloadMaterials", &materialApi, &MaterialApi::ReloadMaterials)

                .addObjectFunction("ReloadShader", &m_ShaderApi, &ShaderApi::ReloadShader)
                .addObjectFunction("ReloadAllShaders", &m_ShaderApi, &ShaderApi::ReloadAllShaders)
                .addObjectFunction("DumpShaders", &m_ShaderApi, &ShaderApi::DumpShaders)
            .endNamespace()
        .endNamespace()
        ;
    }
protected:
    RendererFacade* m_RendererFacade = nullptr;
    ShaderApi m_ShaderApi;
    TextureApi m_TextureApi;
    ContextApi m_ContextApi;
    MaterialApi materialApi;
    HandleApi hadleApi;
};

//---------------------------------------------------------------------------------------

ScriptApi::ScriptApi(RendererFacade * facade) {
    m_Impl = std::make_unique<ScriptApiImpl>(facade);
}

ScriptApi::~ScriptApi() {
}

void ScriptApi::Install(lua_State *lua) {
    assert(lua);
    assert(m_Impl);
    m_Impl->Install(lua);
}

} //namespace MoonGlare::Renderer 
