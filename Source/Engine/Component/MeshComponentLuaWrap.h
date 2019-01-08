#pragma once

#include <Foundation/Component/ComponentInfo.h>
#include <Foundation/Component/ComponentArray.h>
#include <Foundation/Component/ComponentScriptWrapBase.h>
#include <Foundation/Scripts/LuaPanic.h>

#include "SkinComponent.h"

namespace MoonGlare::Component {

/*@ [ComponentReference/MeshComponentLuaWrap] Mesh component
    Component is responsible for rendering mesh @*/    
struct MeshComponentLuaWrap : public ComponentScriptWrapTemplate<MeshComponent> {     

    static Scripts::ApiInitializer RegisterScriptApi(Scripts::ApiInitializer api) {
        using LuaWrapper = MeshComponentLuaWrap;
        return api
            .deriveClass<LuaWrapper, ComponentScriptWrapBase>("MeshComponent")
/*@ [MeshComponentLuaWrap/_] `MeshComponent.castShadow`
    TODO @*/                
                .addProperty<bool,bool>("castShadow", &GetProp<bool, &MeshComponent::castShadow>, &SetProp<bool, &MeshComponent::castShadow>)
            .endClass()
        ;
    }
};

}       
