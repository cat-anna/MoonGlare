#include "arguments.h"
#include "cli.h"
#include "svfs_lua.h"
#include <cstddef>
#include <embedded/all_files.hpp>
#include <iostream>
#include <list>
#include <memory>
#include <mutex>
#include <orbit_logger/sink/file_sink.h>
#include <string>
#include <vector>

using namespace MoonGlare::Tools::VfsCli;

using OrbitLogger::LogCollector;
using OrbitLogger::StdFileLoggerSink;

int main(int argc, char *argv[]) {
    OrbitLogger::ThreadInfo::SetName("MAIN", true);
    LogCollector::Start();
    LogCollector::AddLogSink<StdFileLoggerSink>("logs/svfs_cli.log");

    InitEnv initenv;
    Parser p;
    if (!p.Run(initenv, argc, argv)) {
        std::cout << "Unable to parse arguments!\n";
        return 1;
    }

    auto lua = Lua::New();

    auto svfs = std::make_unique<SVfsLua>(lua);

    // if (!svfs->Initialize()) {
    // 	printf("Unable to initialize svfs!\n");
    // 	return 1;
    // }

    // SVFS *svfsptr = svfs.get();
    // luabridge::getGlobalNamespace(lua->GetState())
    //     .beginNamespace("inst")
    //     .addVariable<SVFS *>("svfs", &svfsptr, false)
    //     .endNamespace();

    CLI cli(lua);

    if (!lua->Initialize()) {
        printf("Unable to initialize lua vm!\n");
        return 1;
    }

    if (initenv.load_internal_scripts) {
        for (const auto &file : kEmbeddedFiles) {
            // printf("Executing %s\n", file.file_name);
            if (!lua->ExecuteChunk(file.data, file.size, file.file_name)) {
                printf("Unable to load internal script %s!\n", file.file_name);
                return 1;
            }
        }
    }

    if (!lua->ExecuteScriptChunk(initenv.startup_script.c_str(), "initscript"))
        return 1;

    if (initenv.run_cli) {
        if (!cli.Enter(initenv)) {
            std::cout << "Unable to run CLI!\n";
            return 1;
        }
    }

    return 0;
}
