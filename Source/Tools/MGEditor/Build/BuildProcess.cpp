#include PCH_HEADER
#include <icons.h>
#include <Module.h>
#include <boost/algorithm/string.hpp>
#include <boost/process.hpp>
#include <boost/asio.hpp>

#include "BuildProcess.h"

namespace MoonGlare {
namespace Editor {

BuildProcess::BuildProcess(const std::string& id, QtShared::SharedModuleManager moduleManager, BuildSettings Settings)
    : iBackgroundProcess(id, std::move(moduleManager)), settings(std::move(Settings)) {

    readableName = "Module build";
}

void BuildProcess::Run() {
    progress = -1;

    std::vector<StepInfo> steps = {
        {"Preparing", [this] {
            CheckSettings();
            CheckOutputDirectory();
        }},
        { "Preparing base modules", [this] { PrepareBaseModules(); } },
        { "Packing module", [this] { PackModule();} },
        { "Unpack engine binaries", [this] { UnpackEngineBinaries(); } },
        { "Write configuration", [this] { PrepareConfiguration(); } },
    };

    try {
        ExecuteSteps(steps);
    }
    catch (...) {
        throw;
    }
}

void BuildProcess::CheckOutputDirectory() {
    Print("Checking output directory...");

    if (boost::filesystem::exists(settings.outputLocation))
        try {
            boost::filesystem::directory_iterator end_it;
            boost::filesystem::directory_iterator it(settings.outputLocation);
            std::for_each(it, end_it, [](auto arg) { boost::filesystem::remove_all(arg); });
        }
        catch (const std::exception &e) {
            Print("Cannot clear output directory: {}", e.what());
        }

    if (!boost::filesystem::exists(settings.outputLocation))
        boost::filesystem::create_directories(settings.outputLocation);
   
    if (settings.RDCModuleFileName.empty()) {
        settings.RDCModuleFileName = boost::filesystem::path(settings.moduleSourceLocation).parent_path().filename().string();
        settings.RDCModuleFileName += ".rdc";
    }
}

void BuildProcess::PackModule() {
    Print("Packing module...");

    std::list<std::string> command;
    //command.push_back(settings.binLocation + settings.rdccExeName);
    //command.push_back(fmt::format("-v"));
    command.push_back(fmt::format("-i {}", settings.moduleSourceLocation));
    command.push_back(fmt::format("-o {}", settings.outputLocation + "/" + settings.RDCModuleFileName));
    WaitForProcess(settings.binLocation + settings.rdccExeName, command, {}, "[RDC]");

    settings.runtimeModules.push_back(settings.RDCModuleFileName);
}

void BuildProcess::UnpackEngineBinaries() {
    auto engineBinRDC = settings.binLocation + "/Modules/Engine.rdc";
    if (!boost::filesystem::is_regular_file(engineBinRDC)) {
        throw std::runtime_error(fmt::format("Engine rdc is not valid!"));
    }

    std::list<std::string> command;
    command.push_back(fmt::format("-s prompt=nil"));
    //command.push_back(fmt::format("-i {}", settings.moduleSourceLocation));
    //command.push_back(fmt::format("-o {}", settings.outputLocation + "/" + settings.RDCModuleFileName));

    std::list<std::string> input;
    input.push_back(fmt::format("inputRDC = [==[{}]==]", engineBinRDC));
    input.push_back(fmt::format("outputDir = [==[{}]==]", settings.outputLocation + "/"));
    input.push_back(fmt::format("mount(inputRDC, [[/]])"));
    input.push_back(fmt::format("exporter = Register:CreateExporter([[FSExporter]])"));
    input.push_back(fmt::format("exporter:DoExport([[/]], outputDir)"));
    input.push_back(fmt::format("io.flush()"));
    input.push_back(fmt::format("os.exit(0)"));
    
    WaitForProcess(settings.binLocation + settings.svfsExeName, command, input, "[SVFS]");
}

void BuildProcess::PrepareBaseModules() {
    auto processModule = [this](const std::string &name) {
        Print("Processing module {}", name);
        settings.runtimeModules.push_back(name);
        auto fullName = settings.binLocation + "/Modules/" + name;
        if (!boost::filesystem::is_regular_file(fullName)) {
            throw std::runtime_error(fmt::format("RDC is not valid: {}", name));
        }
        boost::filesystem::copy_file(fullName, settings.outputLocation + "/" + name);
    };
    processModule("Base.rdc");
    if(settings.debugBuild)
        processModule("Debug.rdc");
}

void BuildProcess::PrepareConfiguration() {
    {
        static const std::string_view moduleListFileName = "ModuleList.txt";
        std::ofstream file(settings.outputLocation + moduleListFileName.data(), std::ios::out);
        file << "# this character makes a comment\n";
        file << "# this file cant be modified\n";
        file << "# but Base.rdc module has to be first\n";
        for (auto &line : settings.runtimeModules)
            file << line << "\n";
    }
}

void BuildProcess::WaitForProcess(const std::string &command, const std::list<std::string> &arguments, const std::list<std::string> &inputLines, const std::string &logMarker) {
    namespace bp = boost::process;

    boost::asio::io_service ios;
    bp::async_pipe ap(ios);
    bp::opstream in;
    
    auto str = boost::join(arguments, std::string(" "));
    bp::child c;
    try {
        c = bp::child(command + " " + str,  bp::std_out > ap, bp::std_in < in, bp::std_err > ap);
    }                                     
    catch (...) {
        ap.close();
        throw;
    }

    std::thread feedThread;

    if (!inputLines.empty()) {
        feedThread = std::thread([&]() {
            for (const auto &line : inputLines)
                in << line << "\n" << std::flush;
        });
    }

    boost::asio::streambuf b;
    std::function<void()> doRead;
    auto handler = [&](const boost::system::error_code &ec, std::size_t size) {
        if (ec)
            return;

        std::string str{
            boost::asio::buffers_begin(b.data()),
            boost::asio::buffers_begin(b.data()) + size };
        b.consume(size);

        std::vector<std::string> lines;
        boost::split(lines, str, boost::is_any_of("\n"));
        for (auto &line : lines) {
            boost::trim(line);
            if (!line.empty())
                Print(fmt::format("{} {}", logMarker, line));
        }
        doRead();
    };
    doRead = [&]() {
        boost::asio::async_read_until(ap, b, '\n', handler);
    };

    doRead();

    ios.run();
    c.wait();
    if(feedThread.joinable())
        feedThread.join();
    if(ap.is_open())
        ap.close();
    int result = c.exit_code();

    if (result != 0) {
        //print(fmt::format("svfs failed with code {}", result), true);
        throw "process failed!";
    }

    //print(fmt::format("svfs ended with code {}", result));
}

void BuildProcess::CheckSettings() {
    Print("Checking build settings...");
    boost::replace_all(settings.moduleSourceLocation, "\\", "/");
    boost::replace_all(settings.outputLocation, "\\", "/");

#ifdef WINDOWS
    settings.rdccExeName += ".exe";
    settings.svfsExeName += ".exe";
#endif
}

} //namespace Editor 
} //namespace MoonGlare 
