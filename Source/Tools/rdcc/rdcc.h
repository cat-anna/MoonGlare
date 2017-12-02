#pragma once

#include <list>
#include <string>
#include <variant>
#include <memory>

#include "config.h"

namespace MoonGlare::RDCC {

struct InputFileInfo {
    std::string globalPath;
    std::string localPath;
    std::string extension;
};

struct ProcessedFileInfo {
    InputFileInfo inputFileInfo;

    enum class DataSourceMode { Unknown, File, String, TempFile, };
    std::string fileDataSource;
    DataSourceMode dataSourceMode = DataSourceMode::Unknown;

    bool compiled = false;
};

class RDCC {
public:
    RDCC(RDCCConfig Config);

    int Run();
private:                            
    RDCCConfig config;

    std::list<InputFileInfo> DoFileSearch();
    std::list<ProcessedFileInfo> ProcessFiles(const std::list<InputFileInfo> &inputFiles);
    ProcessedFileInfo ProcessFile(const InputFileInfo & inputFileInfo);

    std::list<std::string> GenerateSVFSScript(const std::list<ProcessedFileInfo> &processedFiles);
    void ExecuteSVFS(const std::list<std::string> &script);
    void CleanUp(std::list<ProcessedFileInfo>);

    void print(std::string msg, bool ignoreVerbose = false) const ;
};

}