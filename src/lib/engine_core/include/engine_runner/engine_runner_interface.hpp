#pragma once

#include <stop_interface.hpp>
#include <string>

namespace MoonGlare {

class iEngineRunner : public iStopInterface {
public:
    virtual ~iEngineRunner() = default;

    // virtual std::string ApplicationPath() const = 0;
    // virtual std::string SettingsPath() const;

    // bool IsActive() const { return m_Flags.m_Active; }
    virtual bool WantsSoftRestart() const = 0;
    virtual void SetSoftRestart(bool v) = 0;

    virtual std::string GetVersionString() const { return "?"; }
    virtual std::string GetCompilationDate() const { return "?"; }
};

} //namespace MoonGlare
