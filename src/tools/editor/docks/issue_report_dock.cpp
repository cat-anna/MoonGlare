#include "issue_report_dock.hpp"
#include "issue_reporter.hpp"
#include <QString>
#include <boost/uuid/uuid_io.hpp>
#include <dock_window_info.hpp>
#include <fmt/format.h>
#include <qt_gui_icons.h>
#include <ui_issue_report_dock.h>

namespace MoonGlare::Tools::Editor::Docks {

struct IssueReportDockInfo : public DockWindowModule<IssueReportDock> {
    IssueReportDockInfo(SharedModuleManager modmgr) : DockWindowModule(std::move(modmgr)) {
        SetSettingId("IssueReportDockModule");
        SetDisplayName(tr("Active issues"));
        SetShortcut("F11");
    }
};

ModuleClassRegister::Register<IssueReportDockInfo> IssueReportDockInfoReg("IssueReportDock");

//----------------------------------------------------------------------------------

static const int IssueRole = Qt::UserRole + 1;
static const int IssueInternalIdRole = Qt::UserRole + 2;

IssueReportDock::IssueReportDock(QWidget *parent, SharedModuleManager smm)
    : DockWindow(parent, smm) {

    SetSettingId("IssueReportDock");
    ui = std::make_unique<Ui::IssueReportDock>();
    ui->setupUi(this);

    view_model = std::make_unique<QStandardItemModel>();
    view_model->setHorizontalHeaderItem(0, new QStandardItem("Type"));
    view_model->setHorizontalHeaderItem(1, new QStandardItem("Group"));
    view_model->setHorizontalHeaderItem(2, new QStandardItem("Message"));
    view_model->setHorizontalHeaderItem(3, new QStandardItem("Line"));
    view_model->setHorizontalHeaderItem(4, new QStandardItem("File"));
    ui->treeView->setModel(view_model.get());
    ui->treeView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->treeView->setColumnWidth(0, 70);
    ui->treeView->setColumnWidth(1, 100);
    ui->treeView->setColumnWidth(2, 400);
    ui->treeView->setColumnWidth(3, 50);
    ui->treeView->setColumnWidth(4, 200);

    auto issue_reporter = GetModuleManager()->QueryModule<iIssueReporter>();
    connect(issue_reporter.get(), &iIssueReporter::IssueCreated, this,
            &IssueReportDock::IssueCreated);
    connect(issue_reporter.get(), &iIssueReporter::IssueRemoved, this,
            &IssueReportDock::IssueRemoved);

    Refresh();
}

IssueReportDock::~IssueReportDock() {
    ui.reset();
}

//----------------------------------------------------------------------------------

void IssueReportDock::Refresh() {
    if (view_model->rowCount() > 0) {
        setWindowTitle(fmt::format("Active issues [{}]", view_model->rowCount()).c_str());
    } else {
        setWindowTitle("Active issues");
    }
}

//----------------------------------------------------------------------------------

void IssueReportDock::IssueCreated(Issue issue) {

    QList<QStandardItem *> cols;
    QStandardItem *view_item;
    cols << (view_item = new QStandardItem());

    switch (issue.type) {
    default:
    case Issue::Type::Unknown:
        view_item->setText("?");
        break;
    case Issue::Type::Error:
        view_item->setText("Error");
        view_item->setBackground(QBrush(QColor(255, 0, 0, 128)));
        break;
    case Issue::Type::Warning:
        view_item->setText("Warning");
        view_item->setBackground(QBrush(QColor(255, 255, 0, 128)));
        break;
    case Issue::Type::Hint:
        view_item->setText("Hint");
        view_item->setBackground(QBrush(QColor(0, 255, 0, 128)));
        break;
    case Issue::Type::Notice:
        view_item->setText("Notice");
        view_item->setBackground(QBrush(QColor(0, 0, 255, 64)));
        break;
    }

    cols << new QStandardItem(issue.group.c_str());
    cols << new QStandardItem(issue.message.c_str());
    {
        std::string txt;
        if (issue.source_line.has_value()) {
            txt += std::to_string(issue.source_line.value_or(0));
            if (issue.source_column.has_value()) {
                txt += ":" + std::to_string(issue.source_column.value_or(0));
            }
        }
        cols << new QStandardItem(txt.c_str());
    }
    cols << new QStandardItem(issue.file_name.value_or("").c_str());

    view_item->setData(QVariant::fromValue(issue), IssueRole);
    view_item->setData(QString::fromStdString(boost::uuids::to_string(issue.id)),
                       IssueInternalIdRole);

    auto root = view_model->invisibleRootItem();
    root->appendRow(cols);
    Refresh();
}

void IssueReportDock::IssueRemoved(Issue issue) {
    auto root = view_model->invisibleRootItem();
    QString iid = QString::fromStdString(boost::uuids::to_string(issue.id));
    for (int row = 0; row < root->rowCount(); ++row) {
        auto ch = root->child(row);
        auto qstr = ch->data(IssueInternalIdRole).toString();
        if (qstr == iid) {
            root->removeRow(row);
            break;
        }
    }
    Refresh();
}
//----------------------------------------------------------------------------------

std::unordered_map<std::string, QWidget *> IssueReportDock::GetStateSavableWidgets() const {
    return {{"treeview", ui->treeView}, {"treeview_header", ui->treeView->header()}};
}

//----------------------------------------------------------------------------------

void IssueReportDock::ShowContextMenu(const QPoint &point) {
}

void IssueReportDock::ItemDoubleClicked(const QModelIndex &) {
}

} // namespace MoonGlare::Tools::Editor::Docks
