#pragma once

#include <QAction>
#include <QIcon>
#include <QWidget>
#include <interface_map.h>
#include <string>
#include <variant>

namespace MoonGlare::Tools {

struct iMainWindowTabsControl;

struct iTabViewBase {
    virtual std::string GetTabTitle() const = 0;
    virtual bool CanClose() const = 0;

    void SetTablController(std::weak_ptr<iMainWindowTabsControl> controller) {
        this->controller = std::move(controller);
    }

protected:
    virtual ~iTabViewBase() = default;
    std::shared_ptr<iMainWindowTabsControl> GetController() const { return controller.lock(); }

private:
    std::weak_ptr<iMainWindowTabsControl> controller;
};

struct iMainWindowTabsControl {
    virtual QWidget *GetTabParentWidget() const = 0;
    virtual void AddTab(const std::string &id, std::shared_ptr<iTabViewBase> tabWidget) = 0;
    virtual bool TabExists(const std::string &id) const = 0;
    virtual std::shared_ptr<iTabViewBase> GetTab(const std::string &id) const = 0;
    virtual void ActivateTab(const std::string &id) = 0;
    virtual void ActivateTab(iTabViewBase *tab_view) = 0;
    virtual void SetTabTitle(iTabViewBase *tab_view, const std::string &title) = 0;
    virtual void SetTabIcon(iTabViewBase *tab_view, QIcon icon) = 0;

protected:
    virtual ~iMainWindowTabsControl() = default;
};

} // namespace MoonGlare::Tools
