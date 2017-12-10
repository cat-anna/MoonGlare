#pragma once

#include <string>
#include <memory>
#include <unordered_map>

#include "common.h"
#include "config.h"

namespace MoonGlare::RDCC {

class OutputCollector : public RDCCBaseClass {
public:
    OutputCollector(std::shared_ptr<RDCCConfig> Config);

    void PassthroughFile(InputFileInfo inputFileInfo);

    const std::list<ProcessedFileInfo>& GetProcessedFilesList() const { return processedFiles; }
protected:
    std::list<ProcessedFileInfo> processedFiles;
};

}
