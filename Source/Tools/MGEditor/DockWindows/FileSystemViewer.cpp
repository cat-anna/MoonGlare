/*
  * Generated by cppsrc.sh
  * On 2016-07-22  0:08:40,44
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/
#include PCH_HEADER
#include "FileSystemViewer.h"
#include <ui_FileSystemViewer.h>

#include <DockWindowInfo.h>
#include <iFileProcessor.h>
#include <iFileIconProvider.h>

#include <icons.h>

#include "../Windows/MainWindow.h"
#include "../FileSystem.h"

namespace MoonGlare {
namespace Editor {
namespace DockWindows {

struct FileSystemViewerInfo 
		: public QtShared::DockWindowInfo
		, public QtShared::iEditorInfo {
	virtual std::shared_ptr<QtShared::DockWindow> CreateInstance(QWidget *parent) override {
		return std::make_shared<FileSystemViewer>(parent);
	}
	FileSystemViewerInfo(QWidget *Parent): QtShared::DockWindowInfo(Parent) {
		SetSettingID("FileSystemViewerInfo");
		SetDisplayName(tr("Filesystem"));
		SetShortcut("F4");
	}
	virtual std::vector<QtShared::FileCreationMethodInfo> GetCreateFileMethods() const override {
		return std::vector<QtShared::FileCreationMethodInfo> {
			QtShared::FileCreationMethodInfo{ "{DIR}", ICON_16_FOLDER_RESOURCE, "Folder", "{DIR}", },
		};
	}
};
QtShared::DockWindowClassRgister::Register<FileSystemViewerInfo> FileSystemViewerInfoReg("FileSystemViewer");

//-----------------------------------------

FileSystemViewer::FileSystemViewer(QWidget * parent) 
	:  QtShared::DockWindow(parent) {
	SetSettingID("FileSystemViewer");
	m_Ui = std::make_unique<Ui::FilesystemViewer>();
	m_Ui->setupUi(this);

	m_FileSystem = MainWindow::Get()->GetFilesystem();

	m_Ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_Ui->treeView, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(ShowContextMenu(const QPoint &)));
	connect(m_Ui->treeView, SIGNAL(doubleClicked(const QModelIndex&)), SLOT(ItemDoubleClicked(const QModelIndex&)));

	connect(Notifications::Get(), SIGNAL(ProjectChanged(Module::SharedDataModule)), this, SLOT(ProjectChanged(Module::SharedDataModule)));
	connect(m_FileSystem.get(), SIGNAL(Changed()), this, SLOT(RefreshFilesystem()));

	m_ViewModel = std::make_unique<QStandardItemModel>();
	m_ViewModel->setSortRole(FileSystemViewerRole::SortString);

	m_Ui->treeView->setModel(m_ViewModel.get());
	m_Ui->treeView->setSelectionMode(QAbstractItemView::SingleSelection);
	m_Ui->treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);

	m_Ui->treeView->setColumnWidth(0, 200);

	QtShared::FileIconProviderClassRegister::GetRegister()->Enumerate([this](auto &ci) {
		auto ptr = ci.SharedCreate();

		for (auto &it : ptr->GetFileIconInfo()) {
			if (it.m_Icon.empty())
				continue;

			if (m_ExtIconMap.find(it.m_Ext) != m_ExtIconMap.end()) {
				if (m_ExtIconMap[it.m_Ext] == it.m_Icon.c_str()) {
					AddLogf(Warning, "Found Another definition for icon for %s", it.m_Ext.c_str());
					continue;
				}
				AddLogf(Error, "Attempt to change icon for %s", it.m_Ext.c_str());
				continue;
			}
			m_ExtIconMap[it.m_Ext] = it.m_Icon.c_str();
			AddLogf(Info, "Registered file icon %s->%s", it.m_Ext.c_str(), it.m_Icon.c_str());
		}
	});
}

FileSystemViewer::~FileSystemViewer() {
	m_Ui.reset();
}

bool FileSystemViewer::DoSaveSettings(pugi::xml_node node) const {
	QtShared::DockWindow::DoSaveSettings(node);
	return true;
}

bool FileSystemViewer::DoLoadSettings(const pugi::xml_node node) {
	QtShared::DockWindow::DoLoadSettings(node);
	return true;
}

bool FileSystemViewer::Create(const std::string & LocationURI, const QtShared::FileCreationMethodInfo & what) {
	QString qname;
	if (!QuerryStringInput("Enter name:", qname))
		return false;

	std::string name = qname.toLocal8Bit().constData();
	std::string URI = LocationURI + name;

	return m_FileSystem->CreateDirectory(URI);
}

void FileSystemViewer::ShowContextMenu(const QPoint &point) {
	QMenu menu;
	QModelIndex index = m_Ui->treeView->indexAt(point);
	auto itemptr = m_ViewModel->itemFromIndex(index);

	bool folder = !index.data(FileSystemViewerRole::IsFile).toBool();
	if (folder) {
		auto *CreateMenu = menu.addMenu(ICON_16_CREATE_RESOURCE, "Create");
		for (auto ptr : MainWindow::Get()->GetSharedData()->m_FileCreators) {
			if (!ptr->m_DockEditor || !dynamic_cast<QtShared::iEditor*>(ptr->m_DockEditor->GetInstance().get()))
				continue;
			CreateMenu->addAction(QIcon(ptr->m_Info.m_Icon.c_str()), ptr->m_Info.m_Caption.c_str(), [this, ptr, index]() {
				auto qstr = index.data(FileSystemViewerRole::FileURI).toString();
				std::string URI = qstr.toLocal8Bit().constData();
				MainWindow::Get()->CreateFileEditor(URI, ptr);
			});
		}

	} else {
		menu.addAction("Open", this, &FileSystemViewer::OpenItem);
	}

	menu.addSeparator();

	if (itemptr) {
		menu.addAction("Copy URI", [itemptr]() {
			auto qstr = itemptr->data(FileSystemViewerRole::FileURI).toString();
			QClipboard *clipboard = QApplication::clipboard();
			clipboard->setText(qstr);
		});
		menu.addAction("Copy hash URI", [itemptr]() {
			auto qstr = itemptr->data(FileSystemViewerRole::FileHash).toString();
			QClipboard *clipboard = QApplication::clipboard();
			clipboard->setText(qstr);
		});
		menu.addSeparator();
	}

	menu.addAction(ICON_16_REFRESH, "Refresh", [this]() {
		m_FileSystem->Reload();
		RefreshFilesystem();
	});

	menu.exec(m_Ui->treeView->mapToGlobal(point));
}

void FileSystemViewer::ItemDoubleClicked(const QModelIndex&) {
	OpenItem();
}

void FileSystemViewer::OpenItem() {
	auto selection = m_Ui->treeView->selectionModel()->currentIndex();
	auto parent = selection.parent();

	auto itemptr = m_ViewModel->itemFromIndex(selection);
	if (!itemptr)
		return;

	std::string fullpath = itemptr->data(FileSystemViewerRole::FileURI).toString().toLocal8Bit().constData();
	MainWindow::Get()->OpenFileEditor(fullpath);
}

void FileSystemViewer::RefreshFilesystem() {
	if (!m_Module) {
	}
	//m_FileSystem->Reload();
	RefreshTreeView();
}

void FileSystemViewer::ProjectChanged(Module::SharedDataModule datamod) {
	m_Module = datamod;
}

void FileSystemViewer::Clear() {
}

void FileSystemViewer::RefreshTreeView() {
	m_ViewModel->clear();
	m_ViewModel->setHorizontalHeaderItem(0, new QStandardItem("File"));

	auto shdata = MainWindow::Get()->GetSharedData();

	decltype(m_VFSItemMap) currmap;
	auto svfs = m_FileSystem->GetVFS();

	StarVFS::HandleEnumerateFunc processitem;
	processitem = [&](StarVFS::FileID fid) -> bool {
		QStandardItem *parent = nullptr;
		QStandardItem *item = nullptr;
		std::swap(m_VFSItemMap[fid], item);

		auto h = svfs->OpenFile(fid);
		if (!h)
			return true;

		if (!item) {
			auto pid = h.GetParrentID();
			parent = currmap[pid];
			if (!parent)
				parent = m_ViewModel->invisibleRootItem();

			QList<QStandardItem*> cols;
			cols << (item = new QStandardItem(h.GetName()));
			parent->appendRow(cols);
		}
		currmap[fid] = item;

		if (h.IsDirectory()) {
			item->setData(ICON_16_FOLDER, Qt::DecorationRole);
			h.EnumerateChildren(processitem);
			item->setData(QVariant(false), FileSystemViewerRole::IsFile);
		} else {
			item->setData(QVariant(true), FileSystemViewerRole::IsFile);
			auto ext = strrchr(h.GetName(), '.');
			if (ext) {
				++ext;
				auto itlocal = m_ExtIconMap.find(ext);
				if (itlocal != m_ExtIconMap.end()) {
					item->setData(QIcon(itlocal->second), Qt::DecorationRole);
				} else {
					auto it = shdata->m_FileIconMap.find(ext);
					if (it != shdata->m_FileIconMap.end()) {
						item->setData(QIcon(it->second.c_str()), Qt::DecorationRole);
					} else
						item->setData(QIcon(), Qt::DecorationRole);
				}
			} else
				item->setData(QIcon(), Qt::DecorationRole);
		}

		auto str = h.GetFullPath();
		if (h.IsDirectory() && str.back() != '/')
			str.push_back('/');
		auto hashuri = StarVFS::MakePathHashURI(h.GetHash());
		auto fileuri = StarVFS::MakeFileURI(str.c_str());
		item->setData(fileuri.c_str(), FileSystemViewerRole::FileURI);
		item->setData(hashuri.c_str(), FileSystemViewerRole::FileHash);
		item->setData(str.c_str(), FileSystemViewerRole::FileFullName);

		std::string sortstring;
		sortstring = h.IsDirectory() ? "D" : "F";
		sortstring += h.GetName();
		item->setData(sortstring.c_str(), FileSystemViewerRole::SortString);

		h.Close();
		
		return true;
	};

	currmap.swap(m_VFSItemMap);
	for (auto &it : currmap)
		delete it.second;

	auto root = svfs->OpenFile("/");
	//processitem(root.GetFID());
	root.EnumerateChildren(processitem);
	root.Close();

	m_ViewModel->sort(0);
}

} //namespace DockWindows 
} //namespace Editor 
} //namespace MoonGlare 
