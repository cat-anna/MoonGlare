#pragma once

#include <Foundation/Component/ComponentInfo.h>
#include <Foundation/Component/ComponentArray.h>
#include <Foundation/Component/ComponentScriptWrapBase.h>
#include <Foundation/Scripts/LuaPanic.h>

#include "SkinComponent.h"

namespace MoonGlare::Component {

struct MeshComponentLuaWrap : public ComponentScriptWrapTemplate<MeshComponent> {     


    static Scripts::ApiInitializer RegisterScriptApi(Scripts::ApiInitializer api) {
        using LuaWrapper = MeshComponentLuaWrap;
        return api
            .deriveClass<LuaWrapper, ComponentScriptWrapBase>("SkinComponent")
                .addProperty<bool,bool>("CastShadow", &GetProp<bool, &MeshComponent::castShadow>, &SetProp<bool, &MeshComponent::castShadow>)
            .endClass()
        ;
    }
};

#if 0

struct MeshComponent::LuaWrapper {
    MeshComponent *component;
    //ScriptComponent *scriptComponent;

    Entity owner;
    mutable ComponentIndex index;

    void Check() const {
        //if (transformComponent->componentIndexRevision != indexRevision) {
        index = component->GetComponentIndex(owner);
        //}
        if (index == ComponentIndex::Invalid) {
            __debugbreak();
            throw Scripts::LuaPanic("Attempt to dereference deleted RectTransform component!");
        }
    }

    bool IsVisible() const {
        Check(); return component->GetEntry(index)->m_Flags.m_Map.m_Visible;
    }
    void SetVisible(bool v) {
        Check(); component->GetEntry(index)->m_Flags.m_Map.m_Visible = v;
    }


    void SetMeshHandle(Renderer::MeshResourceHandle h) {
        Check(); component->GetEntry(index)->meshHandle = h;
    }
    Renderer::MeshResourceHandle GetMeshHandle() const {
        Check(); return component->GetEntry(index)->meshHandle;
    }
    void SetMaterialHandle(Renderer::MaterialResourceHandle h) {
        Check(); component->GetEntry(index)->materialHandle = h;
    }
    Renderer::MaterialResourceHandle GetMaterialHandle() const {
        Check(); return component->GetEntry(index)->materialHandle;
    }
};

MoonGlare::Scripts::ApiInitializer MeshComponent::RegisterScriptApi(MoonGlare::Scripts::ApiInitializer root) {
    return root
        .beginClass<LuaWrapper>("MeshComponent")
        .addProperty("Visible", &LuaWrapper::IsVisible, &LuaWrapper::SetVisible)
        .addProperty("MeshHandle", &LuaWrapper::GetMeshHandle, &LuaWrapper::SetMeshHandle)
        .addProperty("MaterialHandle", &LuaWrapper::GetMaterialHandle, &LuaWrapper::SetMaterialHandle)
        .endClass()
        ;
}

#endif

}       
