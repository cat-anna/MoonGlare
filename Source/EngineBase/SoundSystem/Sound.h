#pragma once

#include <memory>
#include <string>
#include <vector>

#include "iSoundSystem.h"
#include "Decoder/iDecoder.h"

#include "OpenAl.h"

namespace MoonGlare::SoundSystem {

struct SourceState;

class StandaloneSoundPlayer : public iSound {
public:
    StandaloneSoundPlayer(SourceIndex source, SourceState *state);
    virtual ~StandaloneSoundPlayer();
    void Play() override;

protected:
    SourceIndex source = InvalidSourceIndex;
    SourceState *state = nullptr;
};

}
