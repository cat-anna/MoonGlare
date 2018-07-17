#pragma once

#include <qmainwindow.h>
#include <qevent.h>

#include "Module.h"

namespace MoonGlare {
namespace QtShared {

class SubWindow : public QMainWindow {
    Q_OBJECT
public:
    SubWindow(QWidget *parent, QtShared::SharedModuleManager ModuleManager)
        : QMainWindow(parent), moduleManager(ModuleManager)
    {
    }

    ~SubWindow() override {}

    void closeEvent(QCloseEvent * event) override {
        event->accept();
        emit WindowClosed(this);
    }
    void showEvent(QShowEvent * event) override {
        event->accept();
    }
signals:
    void WindowClosed(SubWindow *window);
protected:
    SharedModuleManager GetModuleManager() { return moduleManager; }
    SharedModuleManager GetModuleManager() const { return moduleManager; }
private:
    QtShared::SharedModuleManager moduleManager;
};

}// namespace QtShared 
}// namespace MoonGlare 
