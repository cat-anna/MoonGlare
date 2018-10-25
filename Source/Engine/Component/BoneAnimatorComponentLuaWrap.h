#pragma once

#include <Foundation/Component/ComponentArray.h>
#include <Foundation/Scripts/LuaPanic.h>
#include <Foundation/Resources/SkeletalAnimationManager.h>

#include "BoneAnimatorComponent.h"

namespace MoonGlare::Component {

struct BoneAnimatorComponentLuaWrap {
    ComponentArray *componentArray = nullptr;
    iSubsystemManager *subsystemManager = nullptr;
    Resources::SkeletalAnimationManager *skeletalAnimationManager = nullptr;
    Entity owner;
    mutable BoneAnimatorComponent *componentPtr;

    void Init() {
        subsystemManager->GetInterfaceMap().GetObject(skeletalAnimationManager);
    }

    void Check() const {
        assert(skeletalAnimationManager);
        assert(componentArray);
        //if (transformComponent->componentIndexRevision != indexRevision) {
        //index = component->GetComponentIndex(owner);
        componentPtr = componentArray->Get<BoneAnimatorComponent>(owner);
        //}
        if (componentPtr == nullptr) {
            //if (index == ComponentIndex::Invalid) {
            __debugbreak();
            throw Scripts::LuaPanic("Attempt to dereference non existing SkinComponent component! ");
        }
    }

    void Reset(const char *animSetName) {
        Check();
        skeletalAnimationManager->ResetBlendState(componentPtr->blendState, animSetName, nullptr);
    }

    static Scripts::ApiInitializer RegisterScriptApi(Scripts::ApiInitializer api) {
        using LuaWrapper = BoneAnimatorComponentLuaWrap;
        return api
            .beginClass<LuaWrapper>("BoneAnimatorComponent")
                .addFunction("Reset", &LuaWrapper::Reset)
            //TODO:set localSpeed

        //    .addFunction("Pause", &LuaWrapper::Pause)
        //    .addFunction("Stop", &LuaWrapper::Stop)

        //    .addProperty("State", &LuaWrapper::GetState)
        //    .addProperty("Position", &LuaWrapper::GetPosition)
        //    .addProperty("Duration", &LuaWrapper::GetDuration)

        //    .addProperty("File", &LuaWrapper::GetURI, &LuaWrapper::SetUri)
        //    .addProperty("Loop", &LuaWrapper::GetLoop, &LuaWrapper::SetLoop)
        //    .addProperty("Kind", &LuaWrapper::GetKind, &LuaWrapper::SetKind)
            .endClass()
        ;
    }
};

}
