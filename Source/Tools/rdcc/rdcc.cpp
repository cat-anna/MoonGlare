#include <boost/filesystem.hpp>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <thread>
#include <sstream>
#include <fmt/format.h>
#include <boost/process.hpp>
#include <boost/asio.hpp>
#include <boost/algorithm/string.hpp>

#include "rdcc.h"

namespace MoonGlare::RDCC {

RDCC::RDCC(RDCCConfig Config) :config(Config) {

}

int RDCC::Run() {
    try {
        auto inputFiles = DoFileSearch();
        auto processedFiles = ProcessFiles(inputFiles);
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

    fs::recursive_directory_iterator dir(config.inputDir), end;
    while (dir != end) {
        fs::directory_entry e = *dir;

        if (fs::is_directory(e.path())) {
            if (e.path().filename() == ".editor") {
                dir.no_push(); // don't recurse into this directory.
            }
            continue;
        }

        if (config.fileFilter.has_value()) {
            //TODO:
        }
        InputFileInfo ifi;
        ifi.globalPath = fs::system_complete(e.path()).string();
        ifi.localPath = e.path().string().substr(config.inputDir.length() - 1);
        ifi.extension = e.path().extension().string();
        if(!ifi.extension.empty())
            ifi.extension = ifi.extension.substr(1);//remove "." from beginning

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

std::list<ProcessedFileInfo> RDCC::ProcessFiles(const std::list<InputFileInfo> &inputFiles) {
    std::list<ProcessedFileInfo> output;

    print(fmt::format("Processing {} files", inputFiles.size()));

    for (auto &inputFile : inputFiles) {
        output.emplace_back(ProcessFile(inputFile));
    }

    print("Processing completed");

    return output;
}

ProcessedFileInfo RDCC::ProcessFile(const InputFileInfo & inputFileInfo) {
    ProcessedFileInfo output;
    output.inputFileInfo = inputFileInfo;

    output.dataSourceMode = ProcessedFileInfo::DataSourceMode::File;
    output.fileDataSource = output.inputFileInfo.globalPath;

    print(fmt::format("Processing {}", output.inputFileInfo.localPath));

    return output;
}

std::list<std::string> RDCC::GenerateSVFSScript(const std::list<ProcessedFileInfo> &processedFiles) {
    print("Preparing SVFS script");
    std::list<std::string> output;

    auto put = [this, &output](std::string line) {
        if(config.dumpScript)
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
    if(config.verbose)
        put("\tprint('Adding file: ', v.virtual, ' -> ',  v.system)");
    put("\tinjectFile(v.virtual, v.system)");
    put("end");
    put("");

    put("Exporter = Register:CreateExporter([[RDCExporter]])");
    put(fmt::format("Exporter:DoExport([[/]], [[{}]])", config.outputFile));
    put("");

    print("Done");

    return output;
}

void RDCC::ExecuteSVFS(const std::list<std::string> &script) {
    print("Packing files");
    print(fmt::format("SVFS executable: {}", config.svfsExecutable));

    namespace bp = boost::process;

    boost::asio::io_service ios;
    bp::async_pipe ap(ios);
    bp::opstream in;

    bp::child c;
    try {
        c = bp::child(config.svfsExecutable, "-s prompt=nil", bp::std_out > ap, bp::std_in < in, bp::std_err > ap);
    }
    catch (...) {
        ap.close();
        throw;
    }
                     
    auto feedThread = std::thread([&]() {
        for (const auto &line : script)
            in << line << "\n" << std::flush;

        in << "print([[***DONE***]])\n" << std::flush;
        in << "io.flush()" << "\n" << std::flush;
        in << "os.exit(0)" << "\n" << std::flush;
    });

    boost::asio::streambuf b;
    std::function<void()> doRead;
    auto handler = [&](const boost::system::error_code &ec, std::size_t size) {
        if (ec)
            return;

        std::string str{
            boost::asio::buffers_begin(b.data()),
            boost::asio::buffers_begin(b.data()) + size};
        b.consume(size);

        std::vector<std::string> lines;
        boost::split(lines, str, boost::is_any_of("\n"));
        for (auto &line : lines) {
            boost::trim(line);
            if(!line.empty())
                print(fmt::format("[SVFS] {}", line));
        }

        doRead();
    };
    doRead = [&]() {
            boost::asio::async_read_until(ap, b, '\n', handler);
    };

    doRead();

    ios.run();
    c.wait();
    feedThread.join();
    ap.close();
    int result = c.exit_code();

    if (result != 0) {
        print(fmt::format("svfs failed with code {}", result), true);
        throw "SVFS failed!";
    }
    
    print(fmt::format("svfs ended with code {}", result));
}

void RDCC::CleanUp(std::list<ProcessedFileInfo>) {
    print("Cleaning up");
}

void RDCC::print(std::string msg, bool ignoreVerbose) const
{
    if (!ignoreVerbose && !config.verbose)
        return;

    if (msg.empty() || msg.back() != '\n')
        msg += "\n";

    //auto t = std::time(nullptr);
    //auto tm = *std::localtime(&t);
    std::cout 
        //<< std::put_time(&tm, "%c") << " " 
        << msg << std::flush;
}

}