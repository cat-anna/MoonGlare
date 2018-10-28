#pragma once

#include <Foundation/Component/ComponentInfo.h>
#include <Foundation/Component/ComponentArray.h>
#include <Foundation/Component/ComponentScriptWrapBase.h>
#include <Foundation/Scripts/LuaPanic.h>

#include "SkinComponent.h"

namespace MoonGlare::Component {

struct SkinComponentLuaWrap : public ComponentScriptWrapTemplate<SkinComponent> {

    static Scripts::ApiInitializer RegisterScriptApi(Scripts::ApiInitializer api) {
        using LuaWrapper = SkinComponentLuaWrap;
        return api
            .deriveClass<LuaWrapper, ComponentScriptWrapBase>("SkinComponent")
                .addProperty<bool, bool>("CastShadow", &GetProp<bool, &SkinComponent::castShadow>, &SetProp<bool, &SkinComponent::castShadow>)
            .endClass()
        ;
    }
};

}
