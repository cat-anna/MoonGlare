#pragma once

#include <Foundation/Component/ComponentArray.h>
#include <Foundation/Scripts/LuaPanic.h>

#include "SkinComponent.h"

namespace MoonGlare::Component {

struct SkinComponentLuaWrap {
    //SoundSourceComponent *component;
    Entity owner;
    ComponentArray *componentArray;
    //mutable ComponentIndex index;
    mutable SkinComponent *componentPtr;

    void Check() const {
        //if (transformComponent->componentIndexRevision != indexRevision) {
        //index = component->GetComponentIndex(owner);
        componentPtr = componentArray->Get<SkinComponent>(owner);
        //}
        if (componentPtr == nullptr) {
            //if (index == ComponentIndex::Invalid) {
            __debugbreak();
            throw Scripts::LuaPanic("Attempt to dereference non existing SkinComponent component! ");
        }
    }

    static Scripts::ApiInitializer RegisterScriptApi(Scripts::ApiInitializer api) {
        return api
        //    .beginClass<LuaWrapper>("SoundSourceComponent")
        //    .addFunction("Play", &LuaWrapper::Play)
        //    .addFunction("Pause", &LuaWrapper::Pause)
        //    .addFunction("Stop", &LuaWrapper::Stop)

        //    .addProperty("State", &LuaWrapper::GetState)
        //    .addProperty("Position", &LuaWrapper::GetPosition)
        //    .addProperty("Duration", &LuaWrapper::GetDuration)

        //    .addProperty("File", &LuaWrapper::GetURI, &LuaWrapper::SetUri)
        //    .addProperty("Loop", &LuaWrapper::GetLoop, &LuaWrapper::SetLoop)
        //    .addProperty("Kind", &LuaWrapper::GetKind, &LuaWrapper::SetKind)
        //    .endClass()
        ;
    }
};

}
