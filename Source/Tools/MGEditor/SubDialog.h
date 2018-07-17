#pragma once

#include <qdialog.h>
#include <qevent.h>

#include "Module.h"

namespace MoonGlare {
namespace QtShared {

class SubDialog: public QDialog {
    Q_OBJECT
public:
    SubDialog(QWidget *parent, QtShared::SharedModuleManager ModuleManager)
        : QDialog(parent), moduleManager(ModuleManager)
    {
    }

    ~SubDialog() override {}

    void closeEvent(QCloseEvent * event) override {
        event->accept();
        emit DialogClosed(this);
    }
    void showEvent(QShowEvent * event) override {
        event->accept();
    }
signals:
    void DialogClosed(SubDialog *window);
protected:
    SharedModuleManager GetModuleManager() { return moduleManager; }
    SharedModuleManager GetModuleManager() const { return moduleManager; }
private:
    QtShared::SharedModuleManager moduleManager;
};

}// namespace QtShared 
}// namespace MoonGlare 
