#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include <fmt/format.h>

#include "common.h"
#include "config.h"
#include "OutputCollector.h"

namespace MoonGlare::RDCC {

struct iFileProcessor {
    iFileProcessor(OutputCollector *Output) : output(Output) { 
        config = output->config;
    }

    virtual void ProcessFile(InputFileInfo inputFileInfo) const = 0;
protected:
    OutputCollector *output;
    std::shared_ptr<RDCCConfig> config;

    void print(std::string msg, bool ignoreVerbose = false) const {
        output->print(msg, ignoreVerbose);
    }
};

using SharedFileProcessor = std::shared_ptr<iFileProcessor>;

//------------------------------------------------------------------------

struct PassthroughFileProcessor : public iFileProcessor {
    PassthroughFileProcessor(OutputCollector *Output) : iFileProcessor(Output) { }

    void ProcessFile(InputFileInfo inputFileInfo) const override {
        output->PassthroughFile(std::move(inputFileInfo));
    }
};

//------------------------------------------------------------------------

class FileProcessorDispatcher : RDCCBaseClass {
public:
    FileProcessorDispatcher(std::shared_ptr<RDCCConfig> Config, OutputCollector *outputCollector);
    ~FileProcessorDispatcher();

    void ProcessFile(InputFileInfo inputFileInfo);
private:
    std::unordered_map<std::string, SharedFileProcessor> fileProcessorsByExtension;
    std::unordered_map<std::string, SharedFileProcessor> fileProcessorsByClass;
    SharedFileProcessor unknownTypeFileProcessor;

    OutputCollector *outputCollector;

    template<typename T>
    SharedFileProcessor MakeProcessor() {
        auto p = std::make_shared<T>(outputCollector);
        fileProcessorsByClass[typeid(*p.get()).name()] = p;
        return p;
    }
};

}
