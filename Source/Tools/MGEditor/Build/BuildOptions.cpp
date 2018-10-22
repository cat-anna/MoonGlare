#include PCH_HEADER
#include <qobject.h>
#include <qfiledialog.h>
#include <qabstractbutton.h>
#include "BuildOptions.h"
#include "ui_BuildOptions.h"

#include <qtUtils.h>

#include <icons.h>

namespace MoonGlare {
namespace Editor {

bool has_child(const std::string &path) {
    if (!boost::filesystem::is_directory(path))
        return false;

    boost::filesystem::directory_iterator end_it;
    boost::filesystem::directory_iterator it(path);
    if (it == end_it)
        return false;
    else
        return true;
}

BuildOptions::BuildOptions(QWidget *parent, Module::SharedDataModule dataModule, SharedModuleManager ModuleManager)
	: SubDialog(parent, ModuleManager), dataModule(dataModule)
{
	ui = std::make_unique<Ui::BuildOptions>();
	ui->setupUi(this);
                          
   
    ui->lineEditOutput->setText((dataModule->GetBaseDirectory() + "build/").c_str());

    connect(ui->buttonBox, &QDialogButtonBox::accepted, [this]() {
        if (ui->lineEditOutput->text().isEmpty()) {
            ErrorMessage("Output location field cannot be empty!");
            return;
        }
        if (has_child(ui->lineEditOutput->text().toLocal8Bit().begin()) && !AskForPermission("Selected directory is not empty. It WILL be cleared. Are you sure?")) {
            return;
        }
        accept();
    });
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    QAction *myAction = ui->lineEditOutput->addAction(ICON_16_OPEN, QLineEdit::TrailingPosition);
    connect(myAction, &QAction::triggered, [this]() {
        QString directory = QFileDialog::getExistingDirectory(this, tr("Select output folder"), QDir::currentPath());
        if (!directory.isEmpty()) {
            ui->lineEditOutput->setText(directory);
        }
    });
}

BuildOptions::~BuildOptions() {
	ui.reset();
}

BuildSettings BuildOptions::GetSettings() const {
    BuildSettings bs;

    bs.outputLocation = ui->lineEditOutput->text().toLocal8Bit().begin();

    bs.debugBuild = ui->checkBoxDebug->isChecked();
    bs.disableValidation = ui->checkBoxDisableValidation->isChecked();

    return bs;
}

//----------------------------------------------------------------------------------

} //namespace Editor
} //namespace MoonGlare
