#pragma once

#include <qaction>
#include <variant>

#include <interface_map.h>

namespace MoonGlare::Tools {

struct iActionProvider {
    virtual void __dummyFunction(){};
};

using ActionVariant = std::variant<nullptr_t, QAction *>;

struct iActionBarSink {
    virtual void AddAction(std::string id, ActionVariant action, std::weak_ptr<iActionProvider> provider) = 0;
    virtual void RemoveProvider(std::weak_ptr<iActionProvider> provider) = 0;
};

} // namespace MoonGlare::Tools
