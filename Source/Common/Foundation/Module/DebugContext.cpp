#include "DebugContext.h"

namespace MoonGlare::Module {

DebugContext::DebugContext(InterfaceMap& ifaceMap) {
}

DebugContext::~DebugContext() {   
}

void DebugContext::AddDebugCommand(std::string name, DebugCommand command) { 
    AddLogf(Info, "Added Debug command %s", name.c_str());
    commandMap[name] = std::move(command);
}

const DebugContext::CommandMap& DebugContext::GetCommandMap() const {
    return commandMap;
}

} //namespace MoonGlare::Module
