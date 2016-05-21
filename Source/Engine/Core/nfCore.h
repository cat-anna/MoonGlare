#ifndef NFCORE_H_
#define NFCORE_H_

namespace MoonGlare {
namespace Core {
	struct MoveConfig;
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
}

#include "EntityManager.h"
#include "ComponentManager.h"

#include "Scripts/nfScripts.h"
#include "Scene/nfScene.h"
#include "Objects/nfObjects.h"
#include "Camera/nfCamera.h"

namespace MoonGlare {
namespace Core {

struct MoveConfig { 
	float TimeDelta;

	mutable ::Core::ciScene *Scene = nullptr;
	mutable Graphic::VirtualCamera *Camera = nullptr;
	
//	mutable std::vector<::Core::Scene::ModelInstance*> RenderList;
	mutable std::vector<std::pair<math::mat4, ::DataClasses::ModelPtr>> RenderList;

	bool m_SecondPeriod;
};

}//namespace Core
}//namespace MoonGlare

#endif // NFCORE_H_ 
