#include <fmt/format.h>

#include "FileProcessor.h"

#include "LuaProcessor.h"

namespace MoonGlare::RDCC {

//------------------------------------------------------------------------

FileProcessorDispatcher::FileProcessorDispatcher(std::shared_ptr<RDCCConfig> Config, OutputCollector *outputCollector) : RDCCBaseClass(Config), outputCollector(outputCollector) {
    unknownTypeFileProcessor = MakeProcessor<PassthroughFileProcessor>();    

    auto luaProcessor = MakeProcessor<LuaProcessor>();

    fileProcessorsByExtension["lua"] = luaProcessor;
}

FileProcessorDispatcher::~FileProcessorDispatcher() {
}

void FileProcessorDispatcher::ProcessFile(InputFileInfo inputFileInfo) {

    SharedFileProcessor proc;
    auto procit = fileProcessorsByExtension.find(inputFileInfo.extension);
    if (procit == fileProcessorsByExtension.end()) {
        proc = unknownTypeFileProcessor;
        printf("Cannot find file processor for '%s' ", inputFileInfo.fileName.c_str());
    } else
        proc = procit->second;

    if (!proc)
        throw std::runtime_error(fmt::format("Cannot find processor for file {}", inputFileInfo.globalPath));
    
    proc->ProcessFile(std::move(inputFileInfo));
}

}
