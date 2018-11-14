#pragma once

#include <qaction>
#include <variant>

#include <Foundation/InterfaceMap.h>

namespace MoonGlare::Editor {

struct iTabViewBase {
    virtual std::string GetTabTitle() const = 0;
    virtual bool CanClose() const = 0;
};

struct iMainWindowTabsCtl {
    virtual QWidget* GetTabParentWidget() const = 0;
    virtual void AddTab(const std::string &id, std::shared_ptr<iTabViewBase> tabWidget) = 0;
    virtual bool TabExists(const std::string &id) const = 0;
    virtual void ActivateTabs(const std::string &id) = 0;
};

}
