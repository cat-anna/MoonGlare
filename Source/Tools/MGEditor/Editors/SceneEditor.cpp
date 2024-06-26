/*
 * Generated by cppsrc.sh
 * On 2016-07-22  0:08:40,44
 * by Paweu
 */
/*--END OF HEADER BLOCK--*/
#include PCH_HEADER
#include "SceneEditor.h"

#include "../Windows/MainWindow.h"
#include <DockWindowInfo.h>
#include <FileSystem.h>
#include <icons.h>
#include <ui_SceneEditor.h>

#include <TypeEditor/CustomEditorItemDelegate.h>
#include <iFileIconProvider.h>

namespace MoonGlare {
namespace Editor {
namespace DockWindows {

struct SceneEditorModule : public QtShared::BaseDockWindowModule,
                           public QtShared::iFileIconInfo,
                           public QtShared::iEditorInfo {

    SceneEditorModule(SharedModuleManager modmgr) : BaseDockWindowModule(std::move(modmgr)) {
        SetSettingID("SceneEditor");
        SetDisplayName(tr("Scene editor"));
        SetShortcut("F3");
    }

    virtual std::shared_ptr<QtShared::DockWindow> CreateInstance(QWidget *parent) override {
        return std::make_shared<SceneEditor>(parent, GetModuleManager());
    }

    std::vector<FileIconInfo> GetFileIconInfo() const override {
        return std::vector<FileIconInfo>{
            FileIconInfo{
                "sdx",
                ICON_16_SCENEDESCR_RESOURCE,
            },
        };
    }

    virtual std::vector<FileHandleMethodInfo> GetCreateFileMethods() const override {
        return std::vector<FileHandleMethodInfo>{
            FileHandleMethodInfo{
                "sdx",
                ICON_16_SCENEDESCR_RESOURCE,
                "Scene...",
                "sdx",
            },
        };
    }

    virtual std::vector<FileHandleMethodInfo> GetOpenFileMethods() const override {
        return std::vector<FileHandleMethodInfo>{
            FileHandleMethodInfo{
                "sdx",
                ICON_16_SCENEDESCR_RESOURCE,
                "Edit scene...",
                "sdx",
            },
        };
    }
};
ModuleClassRegister::Register<SceneEditorModule> EntityEditorReg("SceneEditor");

//----------------------------------------------------------------------------------

SceneEditor::SceneEditor(QWidget *parent, SharedModuleManager modmgr)
    : QtShared::DockWindow(parent), iChangeContainer(modmgr) {
    moduleManager.swap(modmgr);

    SetSettingID("SceneEditor");
    m_Ui = std::make_unique<Ui::SceneEditor>();
    m_Ui->setupUi(this);

    m_Ui->EntityTree->SetSettingID("EntityTree");
    m_Ui->treeViewSceneSettings->SetSettingID("SceneSettings");

    SetChangesName("SceneEditor");
    InsertChangesChild(m_Ui->treeViewSceneSettings, "Settings");
    InsertChangesChild(m_Ui->EntityTree, "Entities");

    m_Ui->treeViewSceneSettings->CreateStructure(TypeEditor::Structure::GetStructureInfo("SceneConfiguration"));

    m_Ui->EntityTree->SetModuleManager(moduleManager);
    auto ue = std::make_unique<SceneEntity>();
    m_SceneEntity = ue.get();
    m_Ui->EntityTree->SetEntity(std::move(ue));

    m_ComponentModel = std::make_unique<QStandardItemModel>();
    m_ComponentModel->setHorizontalHeaderItem(0, new QStandardItem("Component tree"));
    m_ComponentModel->setHorizontalHeaderItem(1, new QStandardItem("Values"));
    m_ComponentModel->setHorizontalHeaderItem(2, new QStandardItem("Description"));
    connect(m_ComponentModel.get(), SIGNAL(itemChanged(QStandardItem *)), SLOT(ComponentChanged(QStandardItem *)));
    m_Ui->treeViewComponents->setModel(m_ComponentModel.get());
    m_Ui->treeViewComponents->setSelectionMode(QAbstractItemView::SingleSelection);
    m_Ui->treeViewComponents->setContextMenuPolicy(Qt::CustomContextMenu);
    m_Ui->treeViewComponents->setItemDelegate(new TypeEditor::CustomEditorItemDelegate(moduleManager, this));
    m_Ui->treeViewComponents->setColumnWidth(0, 200);
    m_Ui->treeViewComponents->setColumnWidth(1, 100);
    m_Ui->treeViewComponents->setColumnWidth(2, 100);
}

SceneEditor::~SceneEditor() {
    m_ComponentModel.reset();
    m_ComponentModel.reset();
    m_Ui.reset();
}

//----------------------------------------------------------------------------------

bool SceneEditor::DoSaveSettings(pugi::xml_node node) const {
    SaveColumns(node, "treeViewSceneSettings:Columns", m_Ui->treeViewSceneSettings, 3);
    SaveColumns(node, "treeViewComponents:Columns", m_Ui->treeViewComponents, 3);
    QtShared::DockWindow::DoSaveSettings(node);
    SaveChildSettings(node, m_Ui->EntityTree);
    SaveChildSettings(node, m_Ui->treeViewSceneSettings);
    return true;
}

bool SceneEditor::DoLoadSettings(const pugi::xml_node node) {
    LoadColumns(node, "treeViewSceneSettings:Columns", m_Ui->treeViewSceneSettings, 3);
    LoadColumns(node, "treeViewComponents:Columns", m_Ui->treeViewComponents, 3);
    QtShared::DockWindow::DoLoadSettings(node);
    LoadChildSettings(node, m_Ui->EntityTree);
    LoadChildSettings(node, m_Ui->treeViewSceneSettings);
    return true;
}

void SceneEditor::Clear() {
    m_SceneEntity->Clear();
    // m_Ui->treeViewSceneSettings->
}

//----------------------------------------------------------------------------------

bool SceneEditor::Create(const std::string &LocationURI, const QtShared::iEditorInfo::FileHandleMethodInfo &what) {
    if (!TryCloseData())
        return false;

    std::string name;
    if (!QueryStringInput("Enter name:", name))
        return false;

    std::string URI = LocationURI + name + ".sdx";

    auto fs = moduleManager->QueryModule<FileSystem>();
    if (!fs->CreateFile(URI)) {
        ErrorMessage("Failed during creating sdx file");
        AddLog(Hint, "Failed to create sdx file: " << URI);
        return false;
    }

    m_Ui->treeViewSceneSettings->GetStructure()->ResetToDefault();
    ResetSystemList();

    SetModiffiedState(true);
    m_SceneURI = URI;
    SaveData();
    AddLog(Hint, "created lua: " << URI);

    return true;
}

bool SceneEditor::OpenData(const std::string &URI) {
    if (!TryCloseData())
        return false;

    try {
        pugi::xml_document xdoc;

        auto fs = moduleManager->QueryModule<FileSystem>();
        StarVFS::ByteTable bt;
        if (!fs->GetFileData(URI, bt)) {
            // todo: log sth
            throw false;
        }
        if (bt.byte_size() == 0) {
            throw true;
        }

        if (!xdoc.load_string((char *)bt.get())) {
            // TODO: log sth
            throw false;
        }

        auto root = xdoc.document_element();
        if (!m_SceneEntity->Read(root, "Entities")) {
            AddLog(Hint, "Failed to read sdx Entities: " << URI);
            throw false;
        }

        m_Ui->treeViewSceneSettings->GetStructure()->ResetToDefault();
        if (!m_Ui->treeViewSceneSettings->GetStructure()->Read(root.child("Configuration"))) {
            AddLog(Hint, "Failed to read sdx configuration: " << m_SceneURI);
            throw false;
        }

        ResetSystemList();
        if (!ReadSystemTree(root.child("Systems"))) {
            AddLog(Hint, "Failed to read sdx component tree: " << m_SceneURI);
            throw false;
        }

        m_SceneURI = URI;
        AddLog(Hint, "Opened sdx: " << URI);
        Refresh();
        return true;
    } catch (...) {
        Clear();
        AddLog(Hint, "Failed to open sdx: " << URI);
        m_SystemInstances.clear();
        Refresh();
        return false;
    }
}

bool SceneEditor::SaveData() {
    if (!IsChanged())
        return false;

    try {
        pugi::xml_document xdoc;
        auto root = xdoc.append_child("Scene");

        if (!m_SceneEntity->Write(root, "Entities")) {
            AddLog(Hint, "Failed to write sdx Entities: " << m_SceneURI);
            throw false;
        }

        if (!m_Ui->treeViewSceneSettings->GetStructure()->Write(root.append_child("Configuration"))) {
            AddLog(Hint, "Failed to write sdx configuration: " << m_SceneURI);
            throw false;
        }

        if (!WriteSystemTree(root.append_child("Systems"))) {
            AddLog(Hint, "Failed to write sdx systems tree: " << m_SceneURI);
            throw false;
        }

        std::stringstream ss;
        xdoc.save(ss);
        StarVFS::ByteTable bt;
        bt.from_string(ss.str());
        auto fs = moduleManager->QueryModule<FileSystem>();
        if (!fs->SetFileData(m_SceneURI, bt)) {
            AddLog(Hint, "Failed to write sdx file: " << m_SceneURI);
            throw false;
        }

        SetModiffiedState(false);

        AddLog(Hint, "Saved sdx: " << m_SceneURI);
        return true;
    } catch (...) {
        AddLog(Hint, "Failed to save sdx: " << m_SceneURI);
        return false;
    }
}

bool SceneEditor::TryCloseData() {
    AddLog(Hint, "Trying to close sdx: " << m_SceneURI);
    if (!m_SceneURI.empty() || IsChanged()) {
        if (!AskForPermission("There is a opened scene. Do you want to close it?"))
            return false;
        if (IsChanged() && AskForPermission("Save changes?"))
            SaveData();
    }
    Clear();
    Refresh();
    return true;
}

//----------------------------------------------------------------------------------

void SceneEditor::Refresh() {
    m_Ui->treeViewSceneSettings->Refresh();
    m_Ui->EntityTree->Refresh();
    RefreshSystemTree();
}

//----------------------------------------------------------------------------------

bool SceneEditor::WriteSystemTree(pugi::xml_node node) const {
    for (auto &it : m_SystemInstances) {
        if (!it->m_Enabled)
            continue;

        auto cnode = node.append_child("System");
        cnode.append_attribute("Name") = it->m_SystemInfo->m_Name.c_str();

        if (!it->m_Config)
            continue;

        it->m_Config->Write(cnode);
    }

    return true;
}

bool SceneEditor::ReadSystemTree(const pugi::xml_node node) {
    for (auto &it : m_SystemInstances) {
        it->m_Enabled = false;
    }
    std::list<SharedSystemInstance> clist(m_SystemInstances.begin(), m_SystemInstances.end());
    m_SystemInstances.clear();

    for (auto it = node.first_child(); it; it = it.next_sibling()) {
        const char *name = it.attribute("Name").as_string(nullptr);
        if (!name) {
            AddLogf(Warning, "Found nameless component");
            continue;
        }

        auto ci = TypeEditor::SystemInfo::GetSystemInfo(name);
        if (!ci) {
            AddLogf(Warning, "Found unknown component: %s", name);
            continue;
        }

        auto cinstit =
            std::find_if(clist.begin(), clist.end(), [ci](const SharedSystemInstance &v) { return *v == ci; });
        if (cinstit == clist.end()) {
            continue;
        }
        auto cinst = *cinstit;
        clist.erase(cinstit);

        cinst->m_Enabled = true;
        if (cinst->m_Config)
            cinst->m_Config->Read(it);

        m_SystemInstances.push_back(cinst);
    }

    for (auto it : clist) {
        m_SystemInstances.push_back(it);
    }

    return true;
}

void SceneEditor::ComponentChanged(QStandardItem *item) {
    if (!item)
        return;
    auto *info = item->data(SceneEditorRole::SystemInstance).value<SystemInstance *>();
    if (!info)
        return;

    info->m_Enabled = item->checkState() == Qt::Checked;
    SetModiffiedState(true);
}

//----------------------------------------------------------------------------------

void SceneEditor::RefreshSystemTree() {
    m_ComponentModel->removeRows(0, m_ComponentModel->rowCount());

    auto *root = m_ComponentModel->invisibleRootItem();

    std::map<Core::SubSystemId, QStandardItem *> Deps;
    Deps[(Core::SubSystemId)Core::Component::SubSystemId::Invalid] = root;

    auto GetParent = [&Deps](Core::SubSystemId cid) {
        auto it = Deps.find(cid);
        if (it == Deps.end())
            return Deps[(Core::SubSystemId)Core::Component::SubSystemId::Invalid];
        return it->second;
    };

    for (auto &ptr : m_SystemInstances) {
        auto ci = ptr->m_SystemInfo;
        QStandardItem *Elem = new QStandardItem(ci->m_DisplayName.c_str());

        Elem->setFlags(Elem->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        // em->setData(Qt::CheckStateRole, true);// Qt::Checked);
        Elem->setCheckable(true);
        Elem->setCheckState(ptr->m_Enabled ? Qt::Checked : Qt::Unchecked);

        Elem->setData(QVariant::fromValue(ptr.get()), SceneEditorRole::SystemInstance);

        {
            QList<QStandardItem *> cols;
            cols << Elem;
            cols << new QStandardItem();
            cols << new QStandardItem();
            //	Deps[ci->m_CID] = Elem;
            // GetParent(ci->m_Requirement)
            root->appendRow(cols);
        }

        if (!ptr->m_Config)
            continue;

        QStandardItem *SettingsElem = new QStandardItem("Settings");
        {
            QList<QStandardItem *> cols;
            cols << SettingsElem;
            Elem->appendRow(cols);
        }

        for (auto &value : ptr->m_Config->GetValues()) {
            QStandardItem *CaptionElem = new QStandardItem(value->GetName().c_str());
            CaptionElem->setFlags(CaptionElem->flags() & ~Qt::ItemIsEditable);

            QStandardItem *ValueElem = new QStandardItem();
            ValueElem->setData(value->GetValue().c_str(), Qt::EditRole);
            auto einfoit = TypeEditor::TypeEditorInfo::GetEditor(value->GetTypeName());
            if (einfoit) {
                ValueElem->setData(einfoit->ToDisplayText(value->GetValue()).c_str(), Qt::DisplayRole);
            } else {
                ValueElem->setData(value->GetValue().c_str(), Qt::DisplayRole);
            }

            ValueElem->setData(QVariant::fromValue(value.get()),
                               TypeEditor::CustomEditorItemDelegate::QtRoles::StructureValue);

            QList<QStandardItem *> cols;
            cols << CaptionElem;
            cols << ValueElem;
            SettingsElem->appendRow(cols);
        }
    }
    m_Ui->treeViewComponents->collapseAll();
}

void SceneEditor::ResetSystemList() {
    m_SystemInstances.clear();
    for (auto &it : TypeEditor::SystemInfo::GetSystems()) {
        auto inst = std::make_shared<SystemInstance>();

        inst->m_SystemInfo = it.second;

        inst->m_Config = inst->m_SystemInfo->m_SettingsStructure->m_CreateFunc(nullptr);
        if (inst->m_Config->GetValues().empty())
            inst->m_Config.reset();
        else
            inst->m_Config->ResetToDefault();

        m_SystemInstances.emplace_back(inst);
    }

    // std::sort(m_SystemInstances.begin(), m_SystemInstances.end(), [](auto a, auto b)->bool {
    //	return a->m_ComponentInfo->m_DefautltIndex < b->m_ComponentInfo->m_DefautltIndex;
    //});

    RefreshSystemTree();
}

} // namespace DockWindows
} // namespace Editor
} // namespace MoonGlare
