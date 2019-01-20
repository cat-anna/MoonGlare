#include PCH_HEADER
#include "TreeViewDialog.h"
#include "EditorSettings.h"
#include "ui_TreeViewDialog.h"
#include <qobject.h>

#include <DockWindow.h>
#include <ToolBase/UserQuestions.h>

#include "Notifications.h"

namespace MoonGlare {
namespace Editor {

TreeViewDialog::TreeViewDialog(QWidget *parent, const std::string settingsName)
    : QDialog(parent, Qt::Window)
{
    SetSettingID(settingsName);
    ui = std::make_unique<Ui::TreeViewDialog>();
    ui->setupUi(this);
    
    connect(ui->pushButtonClose, &QPushButton::clicked, [this]() {
        close();
    });

    LoadSettings();
    //ui->treeView->Refresh();
}

TreeViewDialog::~TreeViewDialog() {
    SaveSettings();
    ui.reset();
}

QTreeView* TreeViewDialog::GetTreeView() {
    return ui->treeView;
}

bool TreeViewDialog::DoSaveSettings(pugi::xml_node node) const {
    SaveGeometry(node, this, "Qt:Geometry");
    //SaveChildSettings(node, ui->treeView);
    return true;
}

bool TreeViewDialog::DoLoadSettings(const pugi::xml_node node) {
    LoadGeometry(node, this, "Qt:Geometry");
    //LoadChildSettings(node, ui->treeView);
    return true;
}

//----------------------------------------------------------------------------------

void TreeViewDialog::showEvent(QShowEvent * event) {
    event->accept();
}

void TreeViewDialog::closeEvent(QCloseEvent * event) {
    event->accept();
}

//----------------------------------------------------------------------------------

} //namespace Editor
} //namespace MoonGlare

