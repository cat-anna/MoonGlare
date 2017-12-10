#include <regex>
#include <fmt/format.h>
#include <lua.hpp>

#include "LuaProcessor.h"

#include <Shared/WaitForProcess.h>
#include <libs/LuaWrap/src/LuaException.h>
#include <libs/LuaWrap/src/LuaDeleter.h>

namespace MoonGlare::RDCC {

void LuaProcessor::ProcessFile(InputFileInfo inputFileInfo) const {
    if (!config->validate) {
        output->PassthroughFile(std::move(inputFileInfo));
        return;
    }

    if (!config->compileScripts) {
        CheckSyntax(std::move(inputFileInfo));
        return;
    }
          
    output->PassthroughFile(std::move(inputFileInfo));
    return;

    print(fmt::format("Compiling script {}", inputFileInfo.localPath));

    //string infile = fi.Name + ".luac.in";
    std::string outfile = config->GenTempFile(inputFileInfo.fileName + ".luac");

/*
luac: no input files given
usage: luac [options] [filenames].
Available options are:
  -        process stdin
  -l       list
  -o name  output to file 'name' (default is "luac.out")
  -p       parse only
  -s       strip debug information
  -v       show version information
  --       stop handling options
*/

    std::list<std::string> command;
    command.push_back(config->luacExecutable);
    command.push_back(fmt::format("-o {}", outfile));
    if(!config->debugBuild && !config->dontStripDebugInfo)
        command.push_back(fmt::format("-s"));
    command.push_back(fmt::format("{}", inputFileInfo.globalPath));
    
    auto outF = [this](std::string & line) {
        print(fmt::format("[LUAC] {}", line));
    };

    WaitForProcess(command, {}, outF);

    //check

    //fi.PreloadData();
    //{
    //    std::ofstream f(infile, std::ios::binary | std::ios::out);
    //    f << fi.PreloadedData.get();
    //    f.close();
    //}

    //char buf[256];
    //sprintf(buf, "%s -o %s %s %s",
    //    data.Scripts.ExecFile.c_str(),
    //    outfile.c_str(),
    //    (Scripts::DoNotStripDebugInfo() ? "" : "-s"),
    //    infile.c_str());

    //if (Verbosity::PrintInfo())
    //    cout << data.StageCounter() << "Invoking command: '" << buf << "'\n";
    //system(buf);

    //{
    //    std::ifstream f(outfile, std::ios::binary | std::ios::in);
    //    f.seekg(0, std::ios::end);
    //    if (f.tellg() <= 0 || f.bad() || f.fail()) {
    //        cout << data.StageCounter() << "Script '" << fi.Name << "' has syntax error\n";
    //        fi.ClearPreloaded();
    //        fi.Correct = false;
    //        ++m_InvalidFiles;
    //    }
    //    else {
    //        fi.Correct = true;
    //        if (!Scripts::DoNotCompile()) {
    //            fi.PreloadedSize = f.tellg();
    //            f.seekg(0);
    //            fi.PreloadedData.reset(new char[fi.PreloadedSize + 1]);
    //            fi.PreloadedData.get()[fi.PreloadedSize] = 0;
    //            f.read(fi.PreloadedData.get(), fi.PreloadedSize);
    //            fi.Generated = true;
    //            fi.Name += "c";
    //        }
    //        else {
    //            fi.ClearPreloaded();
    //        }
    //    }
    //}

    //boost::filesystem::remove(infile);
    //boost::filesystem::remove(outfile);

    //if (!fi.Correct)
    //    Error::Error();
}

bool LuaProcessor::CheckSyntax(InputFileInfo inputFileInfo) const {
    print(fmt::format("Checking syntax script {}", inputFileInfo.localPath));

    std::unique_ptr<lua_State, LuaWrap::LuaDeleter> luaPtr;
    luaPtr.reset(luaL_newstate());
    auto lua = luaPtr.get();
    luaopen_base(lua);
    luaopen_math(lua);
    luaopen_bit(lua);
    luaopen_string(lua);
    luaopen_table(lua);
    luaopen_debug(lua);
    lua_atpanic(lua, &LuaWrap::eLuaPanic::ThrowPanicHandler);

    auto ParseError = [&](const std::string &errorstr) {
        //std::regex pieces_regex(R"==(\[(.+)\]\:(\d+)\:\ (.+))==", std::regex::icase);
        //std::smatch pieces_match;
        //if (std::regex_match(errorstr, pieces_match, pieces_regex)) {
            //issue.fileName = m_URI;
            //issue.sourceLine = std::strtol(pieces_match[2].str().c_str(), nullptr, 10);
            //issue.message = pieces_match[3];
            //issue.type = type;
            //issue.group = "Lua";
            std::cout << "ERROR in file " << inputFileInfo.globalPath << "\n";
            std::cout << "Message: " << errorstr << "\n" << std::flush;
        //}
    };

    int result = luaL_loadfile(lua, inputFileInfo.globalPath.c_str());
    switch (result) {
    case 0: {
        //only syntax check is working correctly
        // if (lua_pcall(lua, 0, 0, 0) == 0)
        //     return;
        // std::string errorstr = lua_tostring(lua, -1);
        // ParseError(errorstr, QtShared::Issue::Type::Warning);
        // AddLogf(Hint, "Lua script '%s' error: %s", m_URI.c_str(), errorstr.c_str());
        output->PassthroughFile(std::move(inputFileInfo));
        return true;
    }
    case LUA_ERRSYNTAX: {
        std::string errorstr = lua_tostring(lua, -1);
        ParseError(errorstr);
        return false;
    }
    case LUA_ERRMEM:
        ParseError("Memory allocation failed!");
        return false;
    default:
        ParseError("Unknown error!");
        return false;
    }
}

}
