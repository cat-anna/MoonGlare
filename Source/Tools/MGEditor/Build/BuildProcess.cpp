#include PCH_HEADER
#include <ToolBase/Module.h>
#include <boost/algorithm/string.hpp>
#include <boost/asio.hpp>
#include <boost/process.hpp>
#include <icons.h>


#include "BuildProcess.h"

#include <Foundation/OS/WaitForProcess.h>

namespace MoonGlare {
namespace Editor {

BuildProcess::BuildProcess(const std::string &id, SharedModuleManager moduleManager, BuildSettings Settings)
    : iBackgroundProcess(id, std::move(moduleManager)), settings(std::move(Settings)) {

    readableName = "Module build";
}

void BuildProcess::Run() {
    progress = -1;

    std::vector<StepInfo> steps = {
        {"Preparing",
         [this] {
             CheckSettings();
             CheckOutput();
         }},
    };

    steps.emplace_back(StepInfo{"Packing module", [this] { PackModule(); }});

    if (!settings.RDCPackOnly) {
        steps.emplace_back(StepInfo{"Preparing base modules", [this] { PrepareBaseModules(); }});
        steps.emplace_back(StepInfo{"Unpack engine binaries", [this] { UnpackEngineBinaries(); }});
        steps.emplace_back(StepInfo{"Write configuration", [this] { PrepareConfiguration(); }});
    }

    try {
        ExecuteSteps(steps);
    } catch (...) {
        throw;
    }
}

void BuildProcess::PackModule() {
    Print("Packing module...");

    std::list<std::string> command;
    command.push_back(settings.binLocation + settings.rdccExeName);
    // command.push_back(fmt::format("-v"));
    command.push_back(fmt::format("-i {}", settings.moduleSourceLocation));
    command.push_back(fmt::format("-o {}", settings.outputLocation + "/" + settings.RDCModuleFileName));
    OS::WaitForProcess(command, {}, [this](std::string line) { Print("[RDC] {}", line); });

    settings.runtimeModules.push_back(settings.RDCModuleFileName);
}

void BuildProcess::UnpackEngineBinaries() {
    auto engineBinRDC = settings.binLocation + "/Modules/Engine.rdc";
    if (!boost::filesystem::is_regular_file(engineBinRDC)) {
        throw std::runtime_error(fmt::format("Engine rdc is not valid!"));
    }

    std::list<std::string> command;
    command.push_back(settings.binLocation + settings.svfsExeName);
    command.push_back(fmt::format("-s prompt=nil"));
    // command.push_back(fmt::format("-i {}", settings.moduleSourceLocation));
    // command.push_back(fmt::format("-o {}", settings.outputLocation + "/" + settings.RDCModuleFileName));

    std::list<std::string> input;
    input.push_back(fmt::format("inputRDC = [==[{}]==]", engineBinRDC));
    input.push_back(fmt::format("outputDir = [==[{}]==]", settings.outputLocation + "/"));
    input.push_back(fmt::format("mount(inputRDC, [[/]])"));
    input.push_back(fmt::format("exporter = Register:CreateExporter([[FSExporter]])"));
    input.push_back(fmt::format("exporter:DoExport([[/]], outputDir)"));
    input.push_back(fmt::format("io.flush()"));
    input.push_back(fmt::format("os.exit(0)"));

    OS::WaitForProcess(command, input, [this](std::string line) { Print("[SVFS] {}", line); });
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
    if (settings.debugBuild)
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

    {
        auto isfile = boost::filesystem::path(settings.InputSettingsFile);
        if (boost::filesystem::is_regular_file(isfile)) {
            boost::filesystem::copy_file(isfile, settings.outputLocation + isfile.filename().string());
        }
    }
}

void BuildProcess::CheckSettings() {
    Print("Checking build settings...");
    boost::replace_all(settings.moduleSourceLocation, "\\", "/");
    boost::replace_all(settings.outputLocation, "\\", "/");

    if (settings.RDCPackOnly) {
        if (settings.RDCModuleFileName.empty())
            throw "Output is not specified";
    } else {
        if (settings.InputSettingsFile.empty())
            throw "InputSettings file is not specified";
    }

#ifdef WINDOWS
    settings.rdccExeName += ".exe";
    settings.svfsExeName += ".exe";
#endif
}

void BuildProcess::CheckOutput() {
    Print("Checking output directory...");

    if (boost::filesystem::exists(settings.outputLocation))
        try {
            boost::filesystem::directory_iterator end_it;
            boost::filesystem::directory_iterator it(settings.outputLocation);
            std::for_each(it, end_it, [](auto arg) { boost::filesystem::remove_all(arg); });
        } catch (const std::exception &e) {
            Print("Cannot clear output directory: {}", e.what());
        }

    if (!boost::filesystem::exists(settings.outputLocation))
        boost::filesystem::create_directories(settings.outputLocation);

    if (!settings.RDCPackOnly && settings.RDCModuleFileName.empty()) {
        settings.RDCModuleFileName =
            boost::filesystem::path(settings.moduleSourceLocation).parent_path().filename().string();
        settings.RDCModuleFileName += ".rdc";
    }
}

} // namespace Editor
} // namespace MoonGlare
