#ifndef NFCORE_H_
#define NFCORE_H_

namespace MoonGlare {
namespace Core {
	struct MoveConfig {
		float TimeDelta;
	};

	struct PreRenderConfig {
		Graphic::cRenderDevice &device;

		PreRenderConfig() = delete;
		PreRenderConfig(const PreRenderConfig&) = delete;
		PreRenderConfig& operator=(const PreRenderConfig&) = delete;
	};
}//namespace Core
}//namespace MoonGlare

namespace Core {
	namespace Data {
		class Manager;
	}

	class Engine;
	class Input;
	struct MouseEventDispatcher;

	using ::MoonGlare::Core::MoveConfig;
	using ::MoonGlare::Core::PreRenderConfig;
}

#include "Scripts/nfScripts.h"
#include "Scene/nfScene.h"
#include "Objects/nfObjects.h"
#include "Camera/nfCamera.h"

#endif // NFCORE_H_ 
