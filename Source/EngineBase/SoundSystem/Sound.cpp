#include "Sound.h"

#include "WorkThread.h"

namespace MoonGlare::SoundSystem {

StandaloneSoundPlayer::StandaloneSoundPlayer(SourceIndex source, SourceState *state)
    : source(source), state(state) {
}

StandaloneSoundPlayer::~StandaloneSoundPlayer() {
    if (state) {
        //state->command = SourceCommand::StopPlaying;
        state->releaseOnStop = true;
    }
}

void StandaloneSoundPlayer::Play() {
    //state->command = SourceCommand::ResumePlaying;
    //if (buffer == 0) {
    //    alGenBuffers(1, &buffer);
    //}
    //decoder->Open();
    //decoder->ReadEntireStream(buffer);
    //decoder->Close();

    //if (source == 0) {
    //    alGenSources(1, &source);
    //}

    //alSourcei(source, AL_BUFFER, buffer);
    //alSourcei(source, AL_SOURCE_RELATIVE, AL_TRUE);
    //alSourcePlay(source);
}
}
