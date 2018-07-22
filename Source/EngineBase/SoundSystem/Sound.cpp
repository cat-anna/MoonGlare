#include "Sound.h"

#include "WorkThread.h"

namespace MoonGlare::SoundSystem {

StandaloneSoundPlayer::StandaloneSoundPlayer(SourceIndex source, SourceState *state)
    : source(source), state(state) {

    assert(state);
    state->releaseOnStop = false;
}

StandaloneSoundPlayer::~StandaloneSoundPlayer() {
    state->releaseOnStop = true;
    if(stopOnDestroy)
        state->command = SourceCommand::StopPlaying;
}

void StandaloneSoundPlayer::Play() {
    state->command = SourceCommand::ResumePlaying;
}
void StandaloneSoundPlayer::Pause() {
    state->command = SourceCommand::Pause;
}

void StandaloneSoundPlayer::Stop() {
    state->command = SourceCommand::StopPlaying;
}

float StandaloneSoundPlayer::GetDuration() const {
    return 90;
}

float StandaloneSoundPlayer::GetPosition() const {
    return state->processedSeconds + state->sourceHandle.GetTimePosition();
}

bool StandaloneSoundPlayer::IsPlaying() const {
    return state->status == SourceStatus::Playing;
}

void StandaloneSoundPlayer::SetStopOnDestroy(bool value) {
    stopOnDestroy = value;
}

}
