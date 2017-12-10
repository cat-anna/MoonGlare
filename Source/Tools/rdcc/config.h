#pragma once

#include <optional>
#include <string>

namespace MoonGlare::RDCC {

struct RDCCConfig {
    std::string inputDir;
    std::optional<std::string> fileFilter;
    std::string outputFile;

    bool debugBuild = false;
    bool validate = true;
    bool compileScripts = false;
    bool verbose = false;
    bool dumpScript = false; 
    bool dontStripDebugInfo = true;

    std::string svfsExecutable;
    std::string luacExecutable;
    std::string exeName;
    //std::string exePath;

    std::string tmpPath;

    std::string GenTempFile(const std::string &name = "") {
        auto index = tmpCounter++;
        std::string n = tmpPath + name + "." + std::to_string(index);
        tmpFiles.emplace_back(n);
        return n;
    }

    std::vector<std::string> tmpFiles;
protected:
    uint64_t tmpCounter;
};

struct InputFileInfo {
    std::string globalPath;
    std::string localPath;
    std::string extension;
    std::string fileName;
};

struct ProcessedFileInfo {
    InputFileInfo inputFileInfo;

    enum class DataSourceMode { Unknown, File, String, TempFile, };
    std::string fileDataSource;
    DataSourceMode dataSourceMode = DataSourceMode::Unknown;

    bool compiled = false;
};


}