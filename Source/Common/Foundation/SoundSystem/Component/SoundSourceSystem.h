#pragma once

#include "../HandleApi.h"

#include <Foundation/Component/nfComponent.h>
#include <Foundation/Component/iSubsystem.h>

#include "SoundSourceComponent.h"

namespace MoonGlare::SoundSystem::Component {         
using namespace MoonGlare::Component;

class SoundSourceSystem : public iSubsystem {
public:
    static constexpr char* SystemName = "SoundSourceSystem";
    //static constexpr bool Required = false;
    
    explicit SoundSourceSystem(iSubsystemManager *subsystemManager);
    virtual ~SoundSourceSystem();

    void Step(const SubsystemUpdateData &data) override;
private:

    iSubsystemManager *subsystemManager;
};

}
