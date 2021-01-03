#include "arguments.h"
#include "cli.h"
#include "sound_system_intetration.hpp"
#include "svfs_lua.h"
#include <component/component_info_register.hpp>
#include <cstddef>
#include <embedded/all_files.hpp>
#include <iostream>
#include <list>
#include <memory>
#include <mutex>
#include <orbit_logger/sink/file_sink.h>
#include <orbit_logger/sink/stdout_sink.h>
#include <runtime_modules.h>
#include <string>
#include <tool_base_module_registration.h>
#include <vector>

using namespace MoonGlare::Tools::VfsCli;

using OrbitLogger::LogCollector;
using OrbitLogger::StdFileLoggerSink;
using OrbitLogger::StdOutSink;

int main(int argc, char *argv[]) {
    InitEnv initenv;
    Parser p;
    if (!p.Run(initenv, argc, argv)) {
        std::cout << "Unable to parse arguments!\n";
        return 1;
    }

    MoonGlare::Tools::RegisterBaseModules();
    auto module_manager = MoonGlare::Tools::ModuleManager::CreateModuleManager();
    module_manager->Initialize();
    MoonGlare::Tools::Component::RegisterAllBaseComponents(module_manager);

    OrbitLogger::ThreadInfo::SetName("MAIN", true);
    LogCollector::Start();
    if (initenv.log_file.empty()) {
        LogCollector::AddLogSink<StdOutSink>();
    } else {
        LogCollector::AddLogSink<StdFileLoggerSink>(initenv.log_file);
    }
    LogCollector::SetChannelState(OrbitLogger::LogChannels::Verbose, initenv.verbose);

    auto lua = Lua::New(initenv);
    auto svfs = std::make_unique<SVfsLua>(lua);
    SoundSystemIntegration ssi(lua, svfs.get());
    CLI cli(lua);

    if (!lua->Initialize(module_manager)) {
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

    module_manager->Finalize();
    module_manager.reset();
    LogCollector::Stop();

    return 0;
}
