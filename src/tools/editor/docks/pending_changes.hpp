#pragma once

#include <QStandardItemModel>
#include <change_container.hpp>
#include <dock_window.hpp>
// #include <ToolBase/interfaces/ActionBarSink.h>

namespace Ui {
class PendingChanges;
}

namespace MoonGlare::Tools::Editor::Docks {

struct PendingChangesRole {
    enum {
        ChangePointer = Qt::UserRole + 1,
    };
};

class PendingChanges : public DockWindow {
    Q_OBJECT;

public:
    PendingChanges(QWidget *parent, SharedModuleManager smm);
    virtual ~PendingChanges();

protected:
protected slots:
    void ChangesChanged(std::shared_ptr<iChangeContainer> sender, bool state);
    void Refresh();
    void SaveAll();

    void ShowContextMenu(const QPoint &);
    void ItemDoubleClicked(const QModelIndex &);

private:
    std::unique_ptr<Ui::PendingChanges> ui;
    std::unique_ptr<QStandardItemModel> view_model;
    // std::shared_ptr<iActionProvider> action_provider;
    std::shared_ptr<iChangesManager> changesManager;
};

} // namespace MoonGlare::Tools::Editor::Docks
