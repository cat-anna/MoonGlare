#ifndef NFCORE_H_
#define NFCORE_H_

namespace MoonGlare {
namespace Core {

	struct MoveConfig;

	class HandleTable;
	class InputProcessor;
	class Hooks;	   
	class Console;

	class Engine;

}//namespace Core
}//namespace MoonGlare

namespace Core {
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

const Version::Info& GetMoonGlareEngineVersion();

class TextProcessor;

struct iCustomDraw {
	virtual void DefferedDraw(Graphic::cRenderDevice& dev) { }
	virtual void D2Draw(Graphic::cRenderDevice& dev) { }
protected:
	virtual ~iCustomDraw() { }
};

struct MoveConfig { 
	float TimeDelta;

	Renderer::Frame *m_BufferFrame;

	mutable Renderer::VirtualCamera *Camera = nullptr;
	
	mutable std::vector<iCustomDraw*> CustomDraw;

	mutable std::unique_ptr<Renderer::RenderInput> m_RenderInput;

	bool m_SecondPeriod;
};

}//namespace Core
}//namespace MoonGlare

#endif // NFCORE_H_ 
