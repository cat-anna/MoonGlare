#pragma once

namespace MoonGlare {

class iConsole {
protected:
    virtual ~iConsole() {};
public:
    enum class LineType {
        Regular,
        Highlight,

        Error,
        Warning,
        Hint,
        Debug,

        MaxValue,
    };


    virtual void PushKey(unsigned key) = 0;

    virtual void Clear() = 0;
    virtual void AddLine(const std::string &line, LineType type = LineType::Regular) = 0;

    //virtual void SetVisible(bool value) = 0;
    //virtual bool IsVisible()const = 0;

    virtual void Deactivate() = 0;
    virtual void Activate() = 0;

    virtual bool ProcessConsole(const Core::MoveConfig &config) = 0;
};

}