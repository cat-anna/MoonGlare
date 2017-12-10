#pragma once

#include <memory>
#include <iostream>

#include "config.h"

namespace MoonGlare::RDCC {

class RDCCBaseClass {
public:
    RDCCBaseClass(std::shared_ptr<RDCCConfig> Config) :
            config(std::move(Config))
    {
    }

    std::shared_ptr<RDCCConfig> config;

    void print(std::string msg, bool ignoreVerbose = false) const {
        if (!ignoreVerbose && !config->verbose)
            return;

        if (msg.empty() || msg.back() != '\n')
            msg += "\n";

        //auto t = std::time(nullptr);
        //auto tm = *std::localtime(&t);
        std::cout
            //<< std::put_time(&tm, "%c") << " " 
            << msg << std::flush;
    }
};

}