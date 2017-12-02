#pragma once

#include <optional>
#include <string>

namespace MoonGlare::RDCC {

struct RDCCConfig {
    std::string inputDir;
    std::optional<std::string> fileFilter;
    std::string outputFile;

    bool debugBuild = false;
    bool validate = true;
    bool verbose = false;
    bool dumpScript = false;


    std::string svfsExecutable;
    std::string exeName;
    //std::string exePath;
};

}