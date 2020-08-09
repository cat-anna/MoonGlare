#pragma once

#include "iDebugContext.h"
#include "interface_map.h"
#include <functional>

namespace MoonGlare::Module {

class DebugContext : public iDebugContext {
  public:
    DebugContext(InterfaceMap &ifaceMap);
    ~DebugContext();

    void AddDebugCommand(std::string name, DebugCommand command) override;
    const CommandMap &GetCommandMap() const override;

  private:
    CommandMap commandMap;
};

} // namespace MoonGlare::Module
