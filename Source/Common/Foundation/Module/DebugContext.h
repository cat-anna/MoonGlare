#pragma once

#include <functional>

#include <Foundation/InterfaceMap.h>

#include "iDebugContext.h"

namespace MoonGlare::Module {

class DebugContext : public iDebugContext {
public:
    DebugContext(InterfaceMap& ifaceMap);
    ~DebugContext();

    void AddDebugCommand(std::string name, DebugCommand command) override;
    const CommandMap& GetCommandMap() const override;
private:
    CommandMap commandMap;
};

} //namespace MoonGlare::Module
