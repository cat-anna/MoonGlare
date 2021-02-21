#pragma once

#include "openal_backend.hpp"
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>
#include <memory>
#include <orbit_logger.h>
#include <string>
#include <vector>

namespace MoonGlare::SoundSystem::Backend {

OpenAlBackend::OpenAlBackend() {
    /* Open and initialize a device */
    ALCdevice *device = alcOpenDevice(nullptr);
    ALCcontext *ctx = alcCreateContext(device, nullptr);
    if (ctx == nullptr || alcMakeContextCurrent(ctx) == ALC_FALSE) {
        if (ctx != nullptr) {
            alcCloseDevice(device);
        }

        throw std::runtime_error("Cannot open sound device");
    }

    const ALCchar *name = nullptr;
    if (alcIsExtensionPresent(device, "ALC_ENUMERATE_ALL_EXT")) {
        name = alcGetString(device, ALC_ALL_DEVICES_SPECIFIER);
    }
    if (!name || alcGetError(device) != AL_NO_ERROR) {
        name = alcGetString(device, ALC_DEVICE_SPECIFIER);
    }

    AddLogf(Debug, "OpenAl initialized");
    AddLogf(System, "OpenAl version: %d.%d", alGetInteger(ALC_MAJOR_VERSION), alGetInteger(ALC_MINOR_VERSION));
    AddLogf(System, "Sound device used: %s", name);
    AddLogf(System, "Sound device extensions: %s", alcGetString(device, ALC_EXTENSIONS));
}

OpenAlBackend::~OpenAlBackend() {
    ALCcontext *ctx = alcGetCurrentContext();
    if (ctx == nullptr) {
        return;
    }

    ALCdevice *device = alcGetContextsDevice(ctx);

    alcMakeContextCurrent(nullptr);
    alcDestroyContext(ctx);
    alcCloseDevice(device);
}

} // namespace MoonGlare::SoundSystem::Backend
