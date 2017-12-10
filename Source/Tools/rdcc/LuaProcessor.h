#pragma once

#include <string>
#include <memory>
#include <unordered_map>

#include "common.h"
#include "config.h"
#include "FileProcessor.h"

namespace MoonGlare::RDCC {

class LuaProcessor : public iFileProcessor {
public:
    LuaProcessor(OutputCollector *Output) : iFileProcessor(Output) { }

    void ProcessFile(InputFileInfo inputFileInfo) const override;

protected:
    bool CheckSyntax(InputFileInfo inputFileInfo) const;
};

}
