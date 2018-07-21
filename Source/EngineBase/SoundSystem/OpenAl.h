#pragma once

#include "AL/al.h"
#include "AL/alc.h"
#include "AL/alext.h"

namespace MoonGlare::SoundSystem {

using SoundBuffer = ALuint;
using SoundSource = ALuint
;
using SourceIndex = uint16_t;

static constexpr SoundBuffer InvalidSoundBuffer = 0;
static constexpr SoundSource InvalidSoundSource = 0;
static constexpr SourceIndex InvalidSourceIndex = 0xFFFF;

}