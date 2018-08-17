#pragma once

#include <Foundation/Component/iSubsystem.h>

namespace MoonGlare {
namespace Core {

    struct MoveConfig;
    class InputProcessor;
    class Engine;
    struct RuntimeConfiguration;

}//namespace Core
}//namespace MoonGlare

#include "Scene/nfScene.h"
#include "Component/nfComponent.h"

namespace MoonGlare {
namespace Core {

extern const char *VersionString;
extern const char *ApplicationName;
extern const char *CompilationDate;

struct MoveConfig : MoonGlare::Component::SubsystemUpdateData {
    emath::fvec2 m_ScreenSize;
    Renderer::Frame *m_BufferFrame;
    mutable Renderer::VirtualCamera *Camera = nullptr;
    Graphic::Dereferred::DefferedSink *deferredSink;
    bool m_SecondPeriod;
};

}//namespace Core
}//namespace MoonGlare
