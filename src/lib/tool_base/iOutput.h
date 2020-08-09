#pragma once

#include <list>
#include <memory>
#include <string>
#include <unordered_map>

#include <fmt/format.h>

namespace MoonGlare::Tools {

class iOutput : public std::enable_shared_from_this<iOutput> {
public:
    virtual void Clear() {}
    virtual void PushLine(const std::string &line){};
    virtual void PushLines(const std::list<std::string> &lines) {
        for (auto &l : lines)
            PushLine(l);
    };

    template <typename... ARGS> void Format(ARGS... args) { PushLine(fmt::format(std::forward<ARGS>(args)...)); }
};

using SharedOutput = std::shared_ptr<iOutput>;

class iOutputProvider {
public:
    virtual ~iOutputProvider(){};
    virtual SharedOutput CreateOutput(const std::string &id, const std::string &title) {
        return std::make_shared<iOutput>();
    }
    virtual SharedOutput GetOutput(const std::string &id) { return std::make_shared<iOutput>(); }
    virtual std::unordered_map<std::string, SharedOutput> GetAllOutputs() const { return {}; };
    virtual void Close(const std::string &id) {}
};

} // namespace MoonGlare::Tools
