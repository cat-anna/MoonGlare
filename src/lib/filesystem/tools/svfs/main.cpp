#include <cstddef>
#include <iostream>
#include <list>
#include <memory>
#include <mutex>
#include <string>
#include <vector>
#define BOOST_NUMERIC_CONVERSION_DETAIL_CONVERTER_FLC_12NOV2002_HPP
#define BOOST_NUMERIC_CONVERSION_CONVERTER_FLC_12NOV2002_HPP
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include "arguments.h"
#include "cli.h"
#include "svfs.h"

#include <InstanceProxy.lua.h>
#include <cli.lua.h>
#include <console.lua.h>
#include <help.lua.h>
#include <main.lua.h>
#include <utils.lua.h>
#include <vfs.lua.h>

struct scriptinfo {
    const unsigned char *data;
    const long *len;
    const char *name;
};

static const scriptinfo scripttable[] = {
    {utils_lua, &utils_lua_size, "utils.lua"},
    {help_lua, &help_lua_size, "help.lua"},
    {main_lua, &main_lua_size, "main.lua"},
    {InstanceProxy_lua, &InstanceProxy_lua_size, "InstanceProxy.lua"},
    {console_lua, &console_lua_size, "console.lua"},
    {cli_lua, &cli_lua_size, "cli.lua"},
    {vfs_lua, &vfs_lua_size, "vfs.lua"},
    {},
};

int main(int argc, char **argv) {
#if 0
//	svfs lvfs;
//	if (!lvfs.HandleArguments(argc - 1, argv + 1)) {
//		std::cout << "Unable to handle command line arguments\n";
//		return 1;
//	}
//
//	if (!lvfs.RunCommandLine()) {
//		std::cout << "Unable to start command line\n";
//		return 1;
//	}

	auto lua = Lua::New();

	auto svfs = std::make_unique<SVFS>(lua);
	if (!svfs->Initialize()) {
		printf("Unable to initialize svfs!\n");
		return 1;
	}

    SVFS *svfsptr = svfs.get();
    luabridge::getGlobalNamespace(lua->GetState())
        .beginNamespace("inst")
        .addVariable<SVFS*>("svfs", &svfsptr, false)
        .endNamespace()
        ;

	CLI cli(lua);

	if (!lua->Initialize()) {
		printf("Unable to initialize lua vm!\n");
		return 1;
	}

    for (const scriptinfo *si = scripttable; si->data; ++si) {
        if (!lua->ExecuteChunk(si->data, *si->len, si->name)) {
            printf("Unable to load internal script %s!\n", si->name);
            return 1;
        }
    }

	InitEnv initenv;
	{
		Parser p;
		if (!p.Run(initenv, argc, argv)) {
			std::cout << "Unable to parse arguments!\n";
			return 1;
		}
		if (!lua->ExecuteScriptChunk(p.GetInitScript().c_str(), "initscript"))
			return 1;
	}

	if (initenv.m_RunCLI) {
		if (!cli.Enter(*svfs, initenv)) {
			std::cout << "Unable to run CLI!\n";
			return 1;
		}
	}

#endif
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
