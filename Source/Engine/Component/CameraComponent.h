#pragma once

#include <Renderer/VirtualCamera.h>

namespace MoonGlare::Component {           

struct CameraComponent {
    static constexpr uint32_t ComponentLimit = 32;
    static constexpr char* ComponentName = "Camera";

	union FlagsMap {
		struct MapBits_t {
			bool m_Orthogonal : 1;
		};
		MapBits_t m_Map;
		uint32_t m_UintValue;

		void SetAll() { m_UintValue = 0; m_UintValue = ~m_UintValue; }
		void ClearAll() { m_UintValue = 0; }

		static_assert(sizeof(MapBits_t) <= sizeof(decltype(m_UintValue)), "Invalid Function map elements size!");
	};

	emath::fmat4 m_ProjectionMatrix;
	FlagsMap m_Flags;
	float m_FoV;


    bool Load(ComponentReader &reader, Entity owner);

	void ResetProjectionMatrix(float aspect);
};
//static_assert((sizeof(CameraComponentEntry) % 16) == 0, "Invalid CameraEntry size!");
//static_assert(std::is_pod<LightComponentEntry>::value, "LightComponentEntry must be pod!");

}

