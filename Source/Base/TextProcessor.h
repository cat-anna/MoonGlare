#pragma once

#include <string>

namespace MoonGlare::Resources { class StringTables; }

namespace MoonGlare {

class TextProcessor {
public:
    TextProcessor(Resources::StringTables *Tables = nullptr);
    ~TextProcessor();

    void SetTables(Resources::StringTables *Tables) { stringTables = Tables; }
    void Process(const std::string& input, std::string &out);
protected:
    Resources::StringTables *stringTables;
};

} //namespace MoonGlare 
