#pragma once

#include <string>

#include <interface_map.h>

namespace MoonGlare::Resources {
class StringTables;
}

namespace MoonGlare {

class TextProcessor {
  public:
    TextProcessor(InterfaceMap &ifaceMap);
    ~TextProcessor();

    void SetTables(Resources::StringTables *Tables) { stringTables = Tables; }
    void Process(const std::string &input, std::string &out);

  protected:
    Resources::StringTables *stringTables = nullptr;
};

} // namespace MoonGlare
