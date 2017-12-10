#pragma once

#include <list>
#include <string>
#include <variant>
#include <memory>

#include "config.h"
#include "common.h"
#include "FileProcessor.h"
#include "OutputCollector.h"

namespace MoonGlare::RDCC {

class RDCC : public RDCCBaseClass {
public:
    RDCC(std::shared_ptr<RDCCConfig> Config);

    int Run();
private:                            
    std::unique_ptr<FileProcessorDispatcher> fileProcessorDispatcher;
    std::unique_ptr<OutputCollector> outputCollector;

    std::list<InputFileInfo> DoFileSearch();
    void ProcessFiles(const std::list<InputFileInfo> &inputFiles);

    std::list<std::string> GenerateSVFSScript(const std::list<ProcessedFileInfo> &processedFiles);
    void ExecuteSVFS(const std::list<std::string> &script);
    void CleanUp(std::list<ProcessedFileInfo>);

};

}