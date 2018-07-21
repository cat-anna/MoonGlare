#pragma once

#include <memory>
#include <string>
#include <vector>

#include "iSoundSystem.h"
#include "Decoder/iDecoder.h"

#include "OpenAl.h"

namespace MoonGlare::SoundSystem {

class SourceState;

class StandaloneSoundPlayer : public iSound {
public:
    StandaloneSoundPlayer(SourceIndex source, SourceState *state);
    virtual ~StandaloneSoundPlayer();
    void Play() override;

protected:
    SourceIndex source = 0;
    SourceState *state = nullptr;
};

}
