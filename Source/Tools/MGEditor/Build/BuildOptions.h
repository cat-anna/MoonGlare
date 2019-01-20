#ifndef BUILDOPTIONS_H
#define BUILDOPTIONS_H

#include <SubDialog.h>
#include <ToolBase/UserQuestions.h>

#include "../DataModule.h"
#include "BuildProcess.h"

namespace Ui { class BuildOptions; }

namespace MoonGlare {
namespace Editor {

class BuildOptions
    : public QtShared::SubDialog
    , public UserQuestions
{
    Q_OBJECT
public:
    BuildOptions(QWidget *parent, Module::SharedDataModule dataModule, SharedModuleManager ModuleManager);
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
