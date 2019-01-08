#pragma once

#include <Foundation/Component/ComponentInfo.h>
#include <Foundation/Component/ComponentArray.h>
#include <Foundation/Component/ComponentScriptWrapBase.h>
#include <Foundation/Scripts/LuaPanic.h>

#include "CameraComponent.h"
#include "CameraSystem.h"

namespace MoonGlare::Component {

class CameraSystem;

/*@ [ComponentReference/CameraComponentLuaWrap] Camera component
    Component is responsible for camera handling
@*/    
struct CameraComponentLuaWrap : public ComponentScriptWrapTemplate<CameraComponent> {
    CameraSystem *cameraSystem = nullptr;

    void Init() {
        subsystemManager->GetInterfaceMap().GetObject(cameraSystem);
    }

    static Scripts::ApiInitializer RegisterScriptApi(Scripts::ApiInitializer api) {
        using LuaWrapper = CameraComponentLuaWrap;
        return api
            .beginClass<LuaWrapper>("CameraComponent")
                .addProperty("active", &LuaWrapper::IsActive, &LuaWrapper::Activate)

                //.addProperty<bool, bool>("CastShadow", &GetCastShadows, &SetCastShadows)
                //.addProperty<float, float>("CutOff", &GetProp<float, &LightComponent::cutOff>, &SetProp<float, &LightComponent::cutOff>)
                //.addProperty("Attenuation", &GetAttenuation, &SetAttenuation)
                //.addProperty("Color", &GetColor, &SetColor)
                //.addProperty("AmbientIntensity", &GetAmbientIntensity, &SetAmbientIntensity)
                //.addProperty("DiffuseIntensity", &GetDiffuseIntensity, &SetDiffuseIntensity)
            .endClass()
            ;
    }

    void Activate(bool v) {
        assert(cameraSystem);
        assert(componentArray);
        assert(componentClassId != ComponentClassId::Invalid);
        componentArray->SetActive(owner, v, componentClassId);
        cameraSystem->ActivateCamera(owner);
    }

    //bool GetCastShadows() const { Check(); return componentPtr->base.m_Flags.m_CastShadows; }
    //void SetCastShadows(bool v) { Check(); componentPtr->base.m_Flags.m_CastShadows = v; }
    //math::vec4 GetAttenuation() const { Check(); return componentPtr->attenuation.values; }
    //void SetAttenuation(math::vec4 v) { Check(); componentPtr->attenuation.values = v; }
    //math::vec3 GetColor() const { Check(); return componentPtr->base.m_Color; }
    //void SetColor(math::vec3 v) { Check(); componentPtr->base.m_Color = v; }
    //float GetAmbientIntensity() const { Check(); return componentPtr->base.m_AmbientIntensity; }
    //void SetAmbientIntensity(float v) { Check(); componentPtr->base.m_AmbientIntensity = v; }
    //float GetDiffuseIntensity() const { Check(); return componentPtr->base.m_DiffuseIntensity; }
    //void SetDiffuseIntensity(float v) { Check(); componentPtr->base.m_DiffuseIntensity = v; }
};

}
