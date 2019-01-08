#pragma once

#include <Foundation/Component/ComponentInfo.h>
#include <Foundation/Component/ComponentArray.h>
#include <Foundation/Component/ComponentScriptWrapBase.h>
#include <Foundation/Scripts/LuaPanic.h>

#include "SkinComponent.h"

namespace MoonGlare::Component {

/*@ [ComponentReference/SkinComponentLuaWrap] Skin component
    Component is responsible for animating mesh @*/   
struct SkinComponentLuaWrap : public ComponentScriptWrapTemplate<SkinComponent> {

    static Scripts::ApiInitializer RegisterScriptApi(Scripts::ApiInitializer api) {
        using LuaWrapper = SkinComponentLuaWrap;
        return api
            .deriveClass<LuaWrapper, ComponentScriptWrapBase>("SkinComponent")
/*@ [SkinComponentLuaWrap/_] `SkinComponent.castShadow`
    TODO @*/              
                .addProperty<bool, bool>("castShadow", &GetProp<bool, &SkinComponent::castShadow>, &SetProp<bool, &SkinComponent::castShadow>)
            .endClass()
        ;
    }
};

}
