#pragma once

namespace MoonGlare::Module {

class iDebugContext {
public:
    iDebugContext() = default;
    virtual ~iDebugContext() = default;

    using DebugCommand = std::function<void()>;
    using CommandMap = std::unordered_map<std::string, DebugCommand>;

    virtual void AddDebugCommand(std::string name, DebugCommand command) = 0;
    virtual const CommandMap& GetCommandMap() const = 0;
};

} //namespace MoonGlare::Module
