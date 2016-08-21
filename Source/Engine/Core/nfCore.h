#ifndef NFCORE_H_
#define NFCORE_H_

namespace MoonGlare {
namespace Core {

	struct MoveConfig;

	class HandleTable;
	class InputProcessor;

}//namespace Core
}//namespace MoonGlare

namespace Core {
	class Input;
	struct MouseEventDispatcher;

	using ::MoonGlare::Core::MoveConfig;
}

namespace Graphic {
namespace Light {
	struct LightConfigurationVector;
}
}

#include "EntityManager.h"

#include "Scripts/nfScripts.h"
#include "Scene/nfScene.h"
#include "Objects/nfObjects.h"
#include "Camera/nfCamera.h"

#include "Component/nfComponent.h"

namespace MoonGlare {
namespace Core {

struct MoveConfig { 
	float TimeDelta;

	mutable ciScene *Scene = nullptr;
	mutable Graphic::VirtualCamera *Camera = nullptr;
	
//	mutable std::vector<::Core::Scene::ModelInstance*> RenderList;
	mutable std::vector<std::pair<math::mat4, DataClasses::ModelPtr>> RenderList;
	mutable std::unique_ptr<Graphic::Light::LightConfigurationVector> m_LightConfig;

	bool m_SecondPeriod;
};

}//namespace Core
}//namespace MoonGlare

#endif // NFCORE_H_ 
