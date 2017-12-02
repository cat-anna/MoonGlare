/*
  * Generated by cppsrc.sh
  * On 2016-10-13 18:53:51,09
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#pragma once

#include <iBackgroundProcess.h>
#include <Module.h>

namespace MoonGlare {
namespace Editor {

struct BuildSettings {
    std::string outputLocation;
    std::string moduleSourceLocation;
    
    bool debugBuild = false;
    bool disableValidation = false;  

    std::string RDCModuleFileName;

    std::string binLocation;
    std::string rdccExeName = "rdcc";
    std::string svfsExeName = "svfs";

    std::list<std::string> runtimeModules;
};

class BuildProcess : public QtShared::iBackgroundProcess {
public:
    BuildProcess(const std::string &id, QtShared::SharedModuleManager moduleManager, BuildSettings Settings);
protected:
    BuildSettings settings;

    void CheckSettings();
    void CheckOutputDirectory();
    void PackModule();

    void UnpackEngineBinaries();
    void PrepareBaseModules();
    void PrepareConfiguration();

    void WaitForProcess(const std::string &command, const std::list<std::string> &arguments, const std::list<std::string> &inputLines = {}, const std::string &logMarker = "");

    template<typename ...ARGS>
    void Print(ARGS && ...args) const {
        output->PushLine(fmt::format(std::forward<ARGS>(args)...));
    }

    void Run() override;
};

} //namespace Editor 
} //namespace MoonGlare 
