#pragma once

#include <QStandardItemModel>
#include <dock_window.hpp>
#include <issue_reporter.hpp>

namespace Ui {
class IssueReportDock;
}

namespace MoonGlare::Tools::Editor::Docks {

class IssueReportDock : public DockWindow {
    Q_OBJECT;

public:
    IssueReportDock(QWidget *parent, SharedModuleManager smm);
    virtual ~IssueReportDock();

protected:
    void Refresh();

    void ShowContextMenu(const QPoint &);
    void ItemDoubleClicked(const QModelIndex &);

    void IssueCreated(Issue issue);
    void IssueRemoved(Issue issue);

    std::unordered_map<std::string, QWidget *> GetStateSavableWidgets() const override;

private:
    std::unique_ptr<Ui::IssueReportDock> ui;
    std::unique_ptr<QStandardItemModel> view_model;
};

} // namespace MoonGlare::Tools::Editor::Docks
