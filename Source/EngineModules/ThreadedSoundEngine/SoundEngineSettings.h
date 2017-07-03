
#pragma once

namespace MoonGlare::Modules::ThreadedSoundEngine {

struct SoundEngineSettings {
    bool enabled = true;
    float volumeMaster = 1.0f;
    float volumeMusic = 0.7f;
    float volumeSound = 1.0f;
};

}