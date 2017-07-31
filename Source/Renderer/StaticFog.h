#pragma once

namespace MoonGlare::Renderer {

struct StaticFog {
    bool m_Enabled;
    uint8_t padding[3];
    glm::fvec3 m_Color;
    float m_Start;
    float m_End;

    void Zero() {
        memset(this, 0, sizeof(*this));
    }
};

//static_assert(std::is_trivial_v<StaticFog>);

} //namespace MoonGlare::Renderer
