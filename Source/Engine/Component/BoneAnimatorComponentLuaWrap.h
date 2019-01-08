#pragma once

#include <Foundation/Component/ComponentInfo.h>
#include <Foundation/Component/ComponentArray.h>
#include <Foundation/Component/ComponentScriptWrapBase.h>
#include <Foundation/Scripts/LuaPanic.h>
#include <Foundation/Resources/SkeletalAnimationManager.h>

#include "BoneAnimatorComponent.h"

namespace MoonGlare::Component {

/*@ [ComponentReference/BoneAnimatorComponentLuaWrap] BoneAnimator component
    Component is responsible for skeletal animation @*/    
struct BoneAnimatorComponentLuaWrap : public ComponentScriptWrapTemplate<BoneAnimatorComponent> {
    Resources::SkeletalAnimationManager *skeletalAnimationManager = nullptr;

    void Init() {
        subsystemManager->GetInterfaceMap().GetObject(skeletalAnimationManager);
    }

    void Reset(const char *animSetName) {
        Check();
        skeletalAnimationManager->ResetBlendState(componentPtr->blendState, animSetName, nullptr);
    }

    static Scripts::ApiInitializer RegisterScriptApi(Scripts::ApiInitializer api) {
        using LuaWrapper = BoneAnimatorComponentLuaWrap;
        return api
            .deriveClass<LuaWrapper, ComponentScriptWrapBase>("BoneAnimatorComponent")
/*@ [BoneAnimatorComponentLuaWrap/_] `BoneAnimator:Reset(animationName)`
    TODO @*/               
                .addFunction("Reset", &LuaWrapper::Reset)
            //TODO: change BoneAnimator to base on global time, not timeDelta
            //TODO: set localSpeed(...)
            //TODO: transitionTo(...)
            .endClass()
        ;
    }
};

}
