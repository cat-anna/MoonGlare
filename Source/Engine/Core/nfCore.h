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

namespace MoonGlare {
namespace Renderer {

struct RenderInput;

}
}

#include "EntityManager.h"

#include "Scripts/nfScripts.h"
#include "Scene/nfScene.h"

#include "Component/nfComponent.h"

namespace MoonGlare {
namespace Core {

struct iCustomDraw {
	virtual void DefferedDraw(Graphic::cRenderDevice& dev) { }
	virtual void D2Draw(Graphic::cRenderDevice& dev) { }
protected:
	virtual ~iCustomDraw() { }
};

struct MoveConfig { 
	float TimeDelta;

	mutable ciScene *Scene = nullptr;
	mutable Graphic::VirtualCamera *Camera = nullptr;
	
//	mutable std::vector<::Core::Scene::ModelInstance*> RenderList;
	mutable std::vector<iCustomDraw*> CustomDraw;

	mutable std::unique_ptr<Renderer::RenderInput> m_RenderInput;

	bool m_SecondPeriod;
};

}//namespace Core
}//namespace MoonGlare

#endif // NFCORE_H_ 
