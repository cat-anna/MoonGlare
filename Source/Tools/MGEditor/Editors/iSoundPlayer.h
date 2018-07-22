#pragma once

#include "Module.h"

namespace MoonGlare::Editor {

class iSoundPlayer
{
public:
    virtual ~iSoundPlayer() {}
    virtual void Play(const std::string &uri) = 0;
private:
};

} 

