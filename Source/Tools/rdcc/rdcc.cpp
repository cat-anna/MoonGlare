#include <boost/filesystem.hpp>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <thread>
#include <sstream>
#include <fmt/format.h>
#include <boost/asio.hpp>
#include <boost/algorithm/string.hpp>
#include <Foundation/OS/WaitForProcess.h>

#include "rdcc.h"

namespace MoonGlare::RDCC {

RDCC::RDCC(std::shared_ptr<RDCCConfig> Config) : RDCCBaseClass(Config) {
    outputCollector = std::make_unique<OutputCollector>(Config);
    fileProcessorDispatcher = std::make_unique<FileProcessorDispatcher>(Config, outputCollector.get());
}

int RDCC::Run() {
    try {
        auto inputFiles = DoFileSearch();
        ProcessFiles(inputFiles);
        auto & processedFiles = outputCollector->GetProcessedFilesList();
        auto script = GenerateSVFSScript(processedFiles);
        ExecuteSVFS(script);
        CleanUp(processedFiles);
    }
    catch (const std::exception e) {
        std::cout << e.what() << "\n";
        return 1;
    }
    catch (const char *e) {
        std::cout << e << "\n";
        return 1;
    }

    return 0;
}

std::list<InputFileInfo> RDCC::DoFileSearch() {
    namespace fs = boost::filesystem;

    print("Starting file search");

    std::list<InputFileInfo> inputFileList;

    fs::recursive_directory_iterator dir(config->inputDir), end;
    for (; dir != end; ++dir) {
        fs::directory_entry e = *dir;

        if (fs::is_directory(e.path())) {     
            auto name = e.path().filename().string();
            boost::to_lower(name);
            if (name == ".editor") {
                dir.no_push(); // don't recurse into this directory.
            }
            continue;
        }

        if (config->fileFilter.has_value()) {
            //TODO:
        }
        InputFileInfo ifi;
        ifi.globalPath = fs::system_complete(e.path()).string();
        ifi.localPath = e.path().string().substr(config->inputDir.length() - 1);
        ifi.fileName = e.path().filename().string();
        ifi.extension = e.path().extension().string();
        if (!ifi.extension.empty()) {
            ifi.extension = ifi.extension.substr(1);//remove "." from beginning
            boost::to_lower(ifi.extension);
        }

        auto fixSlash = [](std::string &str) {
            boost::replace_all(str, "\\", "/");
        };

        fixSlash(ifi.globalPath);
        fixSlash(ifi.localPath);

        inputFileList.emplace_back(ifi);
    };

    print(fmt::format("Found {} files", inputFileList.size()));

    if (inputFileList.empty())
        throw "No input files found!";

    return inputFileList;
}

void RDCC::ProcessFiles(const std::list<InputFileInfo> &inputFiles) {
    print(fmt::format("Processing {} files", inputFiles.size()));

    for (auto &inputFile : inputFiles) {
        fileProcessorDispatcher->ProcessFile(inputFile);
    }

    print("Processing completed");
}

std::list<std::string> RDCC::GenerateSVFSScript(const std::list<ProcessedFileInfo> &processedFiles) {
    print("Preparing SVFS script");
    std::list<std::string> output;

    auto put = [this, &output](std::string line) {
        if(config->dumpScript)
            std::cout << line << "\n" << std::flush;

        output.emplace_back(std::move(line));
    };

    //put("HEADER");

    put("");
    put("fileList = {");
    for (auto &file : processedFiles) {
        put(fmt::format("\t{{ virtual=[=[{}]=], system=[=[{}]=], }},", file.inputFileInfo.localPath, file.fileDataSource));
    }
    put("}");
    put("");

    put("for i,v in ipairs(fileList) do");
    if(config->verbose)
        put("\tprint('Adding file: ', v.virtual, ' -> ',  v.system)");
    put("\tinjectFile(v.virtual, v.system)");
    put("end");
    put("");

    put("Exporter = Register:CreateExporter([[RDCExporter]])");
    put(fmt::format("Exporter:DoExport([[/]], [[{}]])", config->outputFile));
    put("");

    print("Done");

    return output;
}

void RDCC::ExecuteSVFS(const std::list<std::string> &script) {
    print("Packing files");
    print(fmt::format("SVFS executable: {}", config->svfsExecutable));


    std::list<std::string> input = script;
    input.push_back("print([[***DONE***]])\n");
    input.push_back("io.flush()\n");
    input.push_back("os.exit(0)\n");

    auto outF = [this](std::string & line) {
        print(fmt::format("[SVFS] {}", line));
    };

    std::list<std::string> command = {
        config->svfsExecutable,
        "-s prompt=nil"
    };

    int result = OS::WaitForProcess(command, input, outF);

    if (result != 0) {
        print(fmt::format("svfs failed with code {}", result), true);
        throw "SVFS failed!";
    }
    
    print(fmt::format("svfs ended with code {}", result));
}

void RDCC::CleanUp(std::list<ProcessedFileInfo>) {
    print("Cleaning up");
    boost::filesystem::remove_all(config->tmpPath);
}

}