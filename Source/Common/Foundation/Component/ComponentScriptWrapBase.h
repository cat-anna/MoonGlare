#pragma once

#include <Foundation/Scripts/ApiInit.h>

#include "ComponentInfo.h"
#include "ComponentArray.h"

namespace MoonGlare::Component {

/*@ [Scripts/ComponentReference] Component api reference 
    All component have their id registered in global `Component` namespace.
@*/
struct ComponentScriptWrapBase {
    ComponentArray *componentArray = nullptr;
    iSubsystemManager *subsystemManager = nullptr;
    ComponentClassId componentClassId = ComponentClassId::Invalid;
    Entity owner = { };

    void SetActive(bool v) {
        assert(componentArray);
        assert(componentClassId != ComponentClassId::Invalid);
        componentArray->SetActive(owner, v, componentClassId);
    }
    bool IsActive() const {
        assert(componentArray);
        assert(componentClassId != ComponentClassId::Invalid);
        return componentArray->IsActive(owner, componentClassId);
    }

    static Scripts::ApiInitializer RegisterScriptApi(Scripts::ApiInitializer api) {
        using LuaWrapper = ComponentScriptWrapBase;
        return api
            .beginClass<ComponentScriptWrapBase>("ComponentScriptWrapBase")
/*@ [ComponentReference/ComponentCommonApi] Common api
    All component handles share some common api  
    * `ComponentHandle.active` set/get component active flag. Inactive components are not processed. 
@*/            
                .addProperty("active", &LuaWrapper::IsActive, &LuaWrapper::SetActive)
            .endClass()
            ;
    }
};

template<typename COMPONENT>
struct ComponentScriptWrapTemplate : public ComponentScriptWrapBase {
    mutable COMPONENT* componentPtr = nullptr;

    void Check() const {
        assert(this);
        assert(componentArray);
        componentPtr = reinterpret_cast<COMPONENT*>(componentArray->Get(owner, componentClassId));
        if (componentPtr == nullptr) {
            __debugbreak();
            throw Scripts::LuaPanic("Attempt to dereference non existing component!");
        }
    }

    template<typename T, T COMPONENT::*PROP>
    void SetProp(T t) {
        Check();
        (componentPtr->*PROP) = t;
    }
    template<typename T, T COMPONENT::*PROP>
    T GetProp() const {
        Check();
        return componentPtr->*PROP;
    }
};

}
