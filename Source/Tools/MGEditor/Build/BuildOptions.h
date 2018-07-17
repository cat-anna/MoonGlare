#ifndef BUILDOPTIONS_H
#define BUILDOPTIONS_H

#include <qtUtils.h>
#include <SubDialog.h>

#include "BuildProcess.h"
#include "../DataModule.h"

namespace Ui { class BuildOptions; }

namespace MoonGlare {
namespace Editor {

class BuildOptions
    : public QtShared::SubDialog
    , public QtShared::UserQuestions
{
    Q_OBJECT
public:
    BuildOptions(QWidget *parent, Module::SharedDataModule dataModule, QtShared::SharedModuleManager ModuleManager);
    ~BuildOptions();
    BuildSettings GetSettings() const;
private:
    std::unique_ptr<Ui::BuildOptions> ui;
    Module::SharedDataModule dataModule;
protected slots:
};

} //namespace Editor
} //namespace MoonGlare

#endif // SETTINGSWINDOW_H
