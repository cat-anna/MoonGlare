#pragma once

#include "config.h"

namespace MoonGlare::RDCC {

struct ArgumentParser {
    ArgumentParser() = delete;
    
    static RDCCConfig Run(int argc, char **argv);
private:
    struct PrivData;
};

}