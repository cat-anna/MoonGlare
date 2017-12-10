#include "OutputCollector.h"

namespace MoonGlare::RDCC {
OutputCollector::OutputCollector(std::shared_ptr<RDCCConfig> Config) : RDCCBaseClass(Config) { }

void OutputCollector::PassthroughFile(InputFileInfo inputFileInfo) {
    ProcessedFileInfo output;
    output.inputFileInfo = std::move(inputFileInfo);
    output.dataSourceMode = ProcessedFileInfo::DataSourceMode::File;
    output.fileDataSource = output.inputFileInfo.globalPath;
    processedFiles.emplace_back(std::move(output));
}

}
