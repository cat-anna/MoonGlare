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

#if 0
	StarVFS::StarVFS vfs;

	printf("Result: %d\n\n\n", vfs.OpenContainer(argc > 1 ? argv[1] : "tcp://127.0.0.1:55555"));
//	printf("Result: %d\n\n\n", vfs.OpenContainer(argc > 2 ? argv[2] : "./core/"));

//	printf("FileTable:\n");
//	vfs.DumpFileTable(std::cout);
	printf("Structure:\n");
	vfs.DumpStructure(std::cout);

	std::cout << std::flush;

	if (argc > 2 && argv[2][0] == '1') {
		vfs.LoadModule<StarVFS::Modules::Remote>();
		while (true)
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

//	auto fid = vfs.FindFile("/premake5.lua");
//	printf("fid: %d\n", fid);
//	
//	printf("\n\n");
//
	auto h = vfs.OpenFile(argc > 3 ? argv[3] : "/premake5.lua");

	printf("path: %s\n", h.GetFullPath().c_str());
	printf("size: %d\n", h.GetSize());
	printf("rwmode: %d\n", h.GetRWMode());

	StarVFS::ByteTable ct;
	if (!h.GetFileData(ct)) {
		printf("\nFailed to read data!\n");
	} else {
		printf("%s\n", ct.get());
	}

#endif
    return 0;
}
