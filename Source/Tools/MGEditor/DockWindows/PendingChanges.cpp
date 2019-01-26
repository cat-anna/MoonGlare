/*
  * Generated by cppsrc.sh
  * On 2016-07-22  0:08:40,44
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/
#include PCH_HEADER
#include "PendingChanges.h"

#include "../Windows/MainWindow.h"
#include <DockWindowInfo.h>
#include <icons.h>
#include <ui_PendingChanges.h>

#include <ToolBase/Module.h>

namespace MoonGlare {
namespace Editor {
namespace DockWindows {

struct PendingChangesInfo 
    : public QtShared::BaseDockWindowModule {

    virtual std::shared_ptr<QtShared::DockWindow> CreateInstance(QWidget *parent) override {
        return std::make_shared<PendingChanges>(parent, GetModuleManager());
    }

    PendingChangesInfo(SharedModuleManager modmgr) : BaseDockWindowModule(std::move(modmgr)) {
        SetSettingID("PendingChanges");
        SetDisplayName(tr("Pending changes"));
        SetShortcut("F12");
    }
};
ModuleClassRegister::Register<PendingChangesInfo> PendingChangesInfoReg("PendingChanges");

//----------------------------------------------------------------------------------

PendingChanges::PendingChanges(QWidget * parent, SharedModuleManager smm)
    :  QtShared::DockWindow(parent, false, std::move(smm)) {
    SetSettingID("PendingChanges");
    m_Ui = std::make_unique<Ui::PendingChanges>(); 
    m_Ui->setupUi(this);

    changesManager = GetModuleManager()->QuerryModule<MoonGlare::Module::ChangesManager>();

    connect(changesManager.get(), &MoonGlare::Module::ChangesManager::Changed, this, &PendingChanges::ChangesChanged);

    m_ViewModel = std::make_unique<QStandardItemModel>();
    m_ViewModel->setHorizontalHeaderItem(0, new QStandardItem("Editor"));
    m_ViewModel->setHorizontalHeaderItem(1, new QStandardItem("Details"));
    m_Ui->treeView->setModel(m_ViewModel.get());
    m_Ui->treeView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_Ui->treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_Ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    m_Ui->treeView->setColumnWidth(0, 100);
    m_Ui->treeView->setColumnWidth(1, 100);
    connect(m_Ui->treeView, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(ShowContextMenu(const QPoint &)));
    connect(m_Ui->treeView, SIGNAL(doubleClicked(const QModelIndex&)), SLOT(ItemDoubleClicked(const QModelIndex&)));

    connect(m_Ui->actionSave_All, &QAction::triggered, this, &PendingChanges::SaveAll);
    connect(m_Ui->actionSave_single, &QAction::triggered, this, &PendingChanges::SaveSingle);
    m_Ui->actionSave_single->setEnabled(false);
    m_Ui->actionSave_All->setEnabled(false);        

    actionProvider = std::make_shared<iActionProvider>();
    auto sink = GetModuleManager()->QuerryModule<iActionBarSink>();
    if (sink) {
        //sink->AddAction("PendingChanges.SaveSingle", m_Ui->actionSave_single, actionProvider);
        sink->AddAction("PendingChanges.SaveAll", m_Ui->actionSave_All, actionProvider);
        sink->AddAction("PendingChanges.Separator", nullptr, actionProvider);
    }
}

PendingChanges::~PendingChanges() {
    auto sink = GetModuleManager()->QuerryModule<iActionBarSink>();
    if (sink) {
        sink->RemoveProvider(actionProvider);
    }
    m_ViewModel.reset();
    m_Ui.reset();
}

//----------------------------------------------------------------------------------

bool PendingChanges::DoSaveSettings(pugi::xml_node node) const {
    QtShared::DockWindow::DoSaveSettings(node);
    return true;
}

bool PendingChanges::DoLoadSettings(const pugi::xml_node node) {
    QtShared::DockWindow::DoLoadSettings(node);
    return true;
}

//----------------------------------------------------------------------------------

void PendingChanges::Refresh() {
    m_ViewModel->removeRows(0, m_ViewModel->rowCount());
    auto root = m_ViewModel->invisibleRootItem();

    bool state = false;
    std::unordered_map<MoonGlare::Module::iChangeContainer*, QStandardItem*> Items;
    Items[nullptr] = root;

    auto GetRoot = [root, &Items](MoonGlare::Module::iChangeContainer* ichg) -> QStandardItem* {
        auto it = Items.find(ichg);
        if (it == Items.end()) {
            return nullptr;
        }
        return it->second;
    };
                                       
    int cnt = 0;
    for (auto &item : changesManager->GetStateMap()) {      
        ++cnt;
        QStandardItem *qitm;
        QList<QStandardItem*> cols;
        cols << (qitm = new QStandardItem(item.first->GetName().c_str()));
        cols << new QStandardItem(item.first->GetInfoLine().c_str());

        auto iroot = GetRoot(item.first->GetParent());
        if (iroot == nullptr) {
            root->appendRow(cols);
        } else {
            Items[item.first] = qitm;
            iroot->appendRow(cols);
        }

        state = true;
    }
    m_Ui->actionSave_All->setEnabled(state);
    //m_Ui->actionSave_single->setEnabled(state);

    if (cnt > 0) {
        setWindowTitle(fmt::format("Pending changes [{}]", m_ViewModel->rowCount()).c_str());
    } else {
        setWindowTitle("Pending changes ");
    }
}

void PendingChanges::SaveSingle() {
    ReportNotImplemented();
}

void PendingChanges::SaveAll() {
    changesManager->SaveAll();
}

//----------------------------------------------------------------------------------

void PendingChanges::ChangesChanged(MoonGlare::Module::iChangeContainer * sender, bool state) {
    Refresh();
}

//----------------------------------------------------------------------------------

void PendingChanges::ShowContextMenu(const QPoint &point) {
//	QMenu menu;
//	QModelIndex index = m_Ui->treeView->indexAt(point);
//
//	if (index.isValid()) {
//
//	}
//
//	menu.addAction("Open", this, &FileSystemViewer::OpenItem);
//	menu.addSeparator();
//	menu.addAction(ICON_16_REFRESH, "Refresh", this, &FileSystemViewer::RefreshFilesystem);
//
//	menu.exec(m_Ui->treeView->mapToGlobal(point));
}

void PendingChanges::ItemDoubleClicked(const QModelIndex&) {
}

void PendingChanges::ProjectChanged(Module::SharedDataModule datamod) {
}

} //namespace DockWindows 
} //namespace Editor 
} //namespace MoonGlare 

