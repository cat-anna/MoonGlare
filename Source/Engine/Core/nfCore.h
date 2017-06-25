#pragma once

namespace MoonGlare {
namespace Core {

    struct MoveConfig;

    class HandleTable;
    class InputProcessor;
    class Hooks;	   

    class Engine;

    struct RuntimeConfiguration;

}//namespace Core
}//namespace MoonGlare

#include "EntityManager.h"

#include "Scripts/nfScripts.h"
#include "Scene/nfScene.h"
#include "Component/nfComponent.h"

namespace MoonGlare {
namespace Core {

const Version::Info& GetMoonGlareEngineVersion();

class TextProcessor;

struct MoveConfig { 
    float TimeDelta;

    emath::fvec2 m_ScreenSize;

    Renderer::Frame *m_BufferFrame;

    mutable Renderer::VirtualCamera *Camera = nullptr;
    
    Graphic::Dereferred::DefferedSink *deferredSink;

    bool m_SecondPeriod;
};

}//namespace Core
}//namespace MoonGlare
