#pragma once

#include <math/EigenMath.h>

namespace MoonGlare::Renderer {

enum class LightType {
    Unknown,
    Point,
    Directional,
    Spot,
};

struct LightAttenuation {
    math::RawVec4 values;

    float Constant() const { return values[0]; }
    float Linear() const { return values[1]; }
    float Exp() const { return values[2]; }
    float Threshold() const { return values[3]; }

    void SetConstant(float v) { values[0] = v; }
    void SetLinear(float v) { values[1] = v; }
    void SetExp(float v) { values[2] = v; }
    void SetThreshold(float v) { values[3] = v; }

    float InfluenceRadius(float ColorFactor) const {
        // float delta = Linear * Linear + 4 * Exp * Constant;
        // float sqrtdelta = sqrtf(delta);
        //
        // float ret = (Linear + sqrtdelta);
        ////if(Exp != 0)
        //	//ret /= 2 * Exp;
        // return ret;
        //(constant - (256.0f / 5.0f) * maxBrightness))
        float ret = (-Linear() + sqrtf(Linear() * Linear() -
                                       4 * Exp() *
                                           (
                                               // Exp() - 256 * ColorFactor
                                               (Constant() - (256.0f) * ColorFactor)))); // - 256 * ColorFactor
        // if (Exp() != 0)
        // ret /= 2 * Exp();
        return ret;
    }

    float LightInfluenceRadius(const math::RGB &Color, float DiffuseIntensity) const {
        return InfluenceRadius(Color.Max() * DiffuseIntensity);
    }
};
static_assert(std::is_pod<LightAttenuation>::value);

struct LightBase {
    math::RGB m_Color;
    float m_AmbientIntensity;
    float m_DiffuseIntensity;

    float MaxIntensity() const { return std::max(m_AmbientIntensity, m_DiffuseIntensity); }

    union {
        struct {
            bool m_CastShadows : 1;
        };
        uint8_t m_UInt8value;
    } m_Flags;
};
static_assert(std::is_pod<LightBase>::value);

struct PointLight {
    LightBase m_Base;
    LightAttenuation m_Attenuation;

    math::RawVec3 m_Position;
    math::RawMat4 m_PositionMatrix;

    float GetLightInfluenceRadius(float scale) const {
        return m_Attenuation.LightInfluenceRadius(m_Base.m_Color, m_Base.MaxIntensity() * scale);
    }
};
static_assert(std::is_pod<PointLight>::value);

struct SpotLight {
    LightBase m_Base;
    LightAttenuation m_Attenuation;
    float m_CutOff;

    math::RawVec3 m_Position;
    math::RawVec3 m_Direction;
    math::RawMat4 m_PositionMatrix;
    math::RawMat4 m_ViewMatrix;

    float GetLightInfluenceRadius(float scale) const {
        return m_Attenuation.LightInfluenceRadius(m_Base.m_Color, m_Base.MaxIntensity() * scale);
    }
};
static_assert(std::is_pod<SpotLight>::value);

// struct DirectionalLight {
//	LightBase m_Base;
//};

using DirectionalLight = LightBase;

static_assert(std::is_pod<DirectionalLight>::value);

} // namespace MoonGlare::Renderer
