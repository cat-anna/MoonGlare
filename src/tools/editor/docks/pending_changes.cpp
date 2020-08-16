#include "pending_changes.hpp"
#include <QMenu>
#include <QVariant>
#include <dock_window_info.hpp>
#include <qt_gui_icons.h>
#include <runtime_modules.h>
#include <ui_pending_changes.h>

namespace MoonGlare::Tools::Editor::Docks {

struct PendingChangesModule : public DockWindowModule<PendingChanges> {
    PendingChangesModule(SharedModuleManager modmgr) : DockWindowModule(std::move(modmgr)) {
        SetSettingId("PendingChangesModule");
        SetDisplayName(tr("Pending changes"));
        SetShortcut("F12");
    }
};

ModuleClassRegister::Register<PendingChangesModule> PendingChangesModuleReg("PendingChanges");

//----------------------------------------------------------------------------------

PendingChanges::PendingChanges(QWidget *parent, SharedModuleManager smm)
    : DockWindow(parent, std::move(smm)) {
    SetSettingId("PendingChangesDock");

    ui = std::make_unique<Ui::PendingChanges>();
    ui->setupUi(this);

    changesManager = GetModuleManager()->QueryModule<iChangesManager>();

    connect(changesManager.get(), &iChangesManager::Changed, this, &PendingChanges::ChangesChanged);

    view_model = std::make_unique<QStandardItemModel>();
    view_model->setHorizontalHeaderItem(0, new QStandardItem("Editor"));
    view_model->setHorizontalHeaderItem(1, new QStandardItem("Details"));
    ui->treeView->setModel(view_model.get());
    ui->treeView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->treeView->setColumnWidth(0, 200);
    ui->treeView->setColumnWidth(1, 100);
    connect(ui->treeView, SIGNAL(customContextMenuRequested(const QPoint &)), this,
            SLOT(ShowContextMenu(const QPoint &)));
    connect(ui->treeView, SIGNAL(doubleClicked(const QModelIndex &)),
            SLOT(ItemDoubleClicked(const QModelIndex &)));

    connect(ui->actionSave_All, &QAction::triggered, this, &PendingChanges::SaveAll);
    ui->actionSave_All->setEnabled(false);

    // action_provider = std::make_shared<iActionProvider>();
    // auto sink = GetModuleManager()->QueryModule<iActionBarSink>();
    // if (sink) {
    //     // sink->AddAction("PendingChanges.SaveSingle", ui->actionSave_single, actionProvider);
    //     sink->AddAction("PendingChanges.SaveAll", ui->actionSave_All, actionProvider);
    //     sink->AddAction("PendingChanges.Separator", nullptr, actionProvider);
    // }
}

PendingChanges::~PendingChanges() {
    // auto sink = GetModuleManager()->QueryModule<iActionBarSink>();
    // if (sink) {
    //     sink->RemoveProvider(actionProvider);
    // }
    view_model.reset();
    ui.reset();
}

//----------------------------------------------------------------------------------

void PendingChanges::Refresh() {
    view_model->removeRows(0, view_model->rowCount());
    auto root = view_model->invisibleRootItem();

    std::unordered_map<std::shared_ptr<iChangeContainer>, QStandardItem *> Items;
    Items[nullptr] = root;

    auto GetRoot = [root, &Items](std::shared_ptr<iChangeContainer> ichg) -> QStandardItem * {
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
        QList<QStandardItem *> cols;
        cols << (qitm = new QStandardItem(std::string(item.first->GetName()).c_str()));
        cols << new QStandardItem(item.first->GetInfoLine().c_str());

        qitm->setData(item.first->GetInfoIcon(), Qt::DecorationRole);
        qitm->setData(QVariant::fromValue(std::weak_ptr<iChangeContainer>(item.first)),
                      PendingChangesRole::ChangePointer);

        auto iroot = GetRoot(item.first->GetParent().lock());
        if (iroot == nullptr) {
            root->appendRow(cols);
        } else {
            Items[item.first] = qitm;
            iroot->appendRow(cols);
        }
    }

    ui->actionSave_All->setEnabled(cnt > 0);
    // ui->actionSave_single->setEnabled(state);

    if (cnt > 0) {
        setWindowTitle(fmt::format("Pending changes [{}]", cnt).c_str());
    } else {
        setWindowTitle("Pending changes ");
    }
}

void PendingChanges::SaveAll() {
    changesManager->SaveAll();
}

//----------------------------------------------------------------------------------

void PendingChanges::ChangesChanged(std::shared_ptr<iChangeContainer> sender, bool state) {
    Refresh();
}

//----------------------------------------------------------------------------------

void PendingChanges::ShowContextMenu(const QPoint &point) {
    QModelIndex index = ui->treeView->indexAt(point);
    if (!index.isValid()) {
        return;
    }

    auto pointer_variant = index.data(PendingChangesRole::ChangePointer);
    auto weak_container = pointer_variant.value<std::weak_ptr<iChangeContainer>>();
    auto container = weak_container.lock();
    if (!container) {
        return;
    }

    QMenu menu;

    auto drop_action = menu.addAction(ICON_16_REMOVE, "Drop", this, [this, container]() {
        if (!container->DropChanges()) {
            ErrorMessage("Drop failed");
        }
    });
    drop_action->setEnabled(container->DropChangesPossible());

    // menu.addSeparator();
    menu.addAction(ICON_16_SAVE_ONE, "Save", [this, container]() {
        if (!container->SaveChanges()) {
            ErrorMessage("Save failed");
        }
    });

    menu.exec(ui->treeView->mapToGlobal(point));
}

void PendingChanges::ItemDoubleClicked(const QModelIndex &index) {
    if (!index.isValid()) {
        return;
    }
    auto pointer_variant = index.data(PendingChangesRole::ChangePointer);
    auto weak_container = pointer_variant.value<std::weak_ptr<iChangeContainer>>();
    auto container = weak_container.lock();
    if (!container) {
        return;
    }
    if (!container->SaveChanges()) {
        ErrorMessage("Save failed");
    }
}

} // namespace MoonGlare::Tools::Editor::Docks
