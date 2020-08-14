#pragma once

#include <list>
#include <string>

namespace MoonGlare::Tools::VfsCli {

struct InitEnv {
    bool run_cli = true;
    bool bash_mode = true;
    bool verbose = false;
    bool load_internal_scripts = true;
    std::string startup_script;
};

struct Parser {
    static bool Run(InitEnv &out, int argc, char **argv);
};

} // namespace MoonGlare::Tools::VfsCli