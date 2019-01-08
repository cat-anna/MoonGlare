#pragma once

#include <Foundation/Component/ComponentInfo.h>
#include <Foundation/Component/ComponentArray.h>
#include <Foundation/Component/ComponentScriptWrapBase.h>
#include <Foundation/Scripts/LuaPanic.h>

#include "LightComponent.h"

namespace MoonGlare::Component {

/*@ [ComponentReference/LightComponentLuaWrap] Light component
    Component is responsible for emitting light @*/    
struct LightComponentLuaWrap : public ComponentScriptWrapTemplate<LightComponent> {

    static Scripts::ApiInitializer RegisterScriptApi(Scripts::ApiInitializer api) {
        using LuaWrapper = LightComponentLuaWrap;
        return api
            .deriveClass<LuaWrapper, ComponentScriptWrapBase>("LightComponent")
/*@ [LightComponentLuaWrap/_] `LightComponent.castShadow`
    TODO @*/                
                .addProperty<bool, bool>("castShadow", &GetCastShadows, &SetCastShadows)
/*@ [LightComponentLuaWrap/_] `LightComponent.cutOff`
    TODO @*/                
                .addProperty<float, float>("cutOff", &GetProp<float, &LightComponent::cutOff>, &SetProp<float, &LightComponent::cutOff>)
/*@ [LightComponentLuaWrap/_] `LightComponent.attenuation`
    TODO @*/                
                .addProperty("attenuation", &GetAttenuation, &SetAttenuation)
/*@ [LightComponentLuaWrap/_] `LightComponent.color`
    TODO @*/                
                .addProperty("color", &GetColor, &SetColor)
/*@ [LightComponentLuaWrap/_] `LightComponent.ambientIntensity`
    TODO @*/                
                .addProperty("ambientIntensity", &GetAmbientIntensity, &SetAmbientIntensity)
/*@ [LightComponentLuaWrap/_] `LightComponent.diffuseIntensity`
    TODO @*/                
                .addProperty("diffuseIntensity", &GetDiffuseIntensity, &SetDiffuseIntensity)
            .endClass()
            ;
    }

    bool GetCastShadows() const { Check(); return componentPtr->base.m_Flags.m_CastShadows; }
    void SetCastShadows(bool v) { Check(); componentPtr->base.m_Flags.m_CastShadows = v; }
    math::vec4 GetAttenuation() const { Check(); return componentPtr->attenuation.values; }
    void SetAttenuation(math::vec4 v) { Check(); componentPtr->attenuation.values = v; }
    math::vec3 GetColor() const { Check(); return componentPtr->base.m_Color; }
    void SetColor(math::vec3 v) { Check(); componentPtr->base.m_Color = v; }
    float GetAmbientIntensity() const { Check(); return componentPtr->base.m_AmbientIntensity; }
    void SetAmbientIntensity(float v) { Check(); componentPtr->base.m_AmbientIntensity = v; }
    float GetDiffuseIntensity() const { Check(); return componentPtr->base.m_DiffuseIntensity; }
    void SetDiffuseIntensity(float v) { Check(); componentPtr->base.m_DiffuseIntensity = v; }
};

}
