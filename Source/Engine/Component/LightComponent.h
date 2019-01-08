#pragma once

#include <Renderer/Light.h>

namespace MoonGlare::Component {

class TransformComponent;

struct LightComponent {
    static constexpr uint32_t ComponentLimit = 1024;
    static constexpr char* ComponentName = "Light";

	Renderer::LightBase base;
	Renderer::LightAttenuation attenuation;
	Renderer::LightType type;

	float cutOff;

    bool Load(ComponentReader &reader, Entity owner);
};

//static_assert((sizeof(MeshEntry) % 16) == 0, "Invalid MeshEntry size!");
//static_assert(std::is_pod<LightComponent>::value, "LightComponent must be pod!");

}
