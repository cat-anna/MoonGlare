#include PCH_HEADER
#include "AudioListWidget.h"
#include "ui_AudioListWidget.h"
#include "mgdtSettings.h"
#include "RemoteConsole.h"
#include "MainForm.h"

class AudioListWidget::LuaRequest : public RemoteConsoleEnumerationObserver {
public:
	LuaRequest(QStandardItem *parent, AudioListWidget *Owner):
			RemoteConsoleEnumerationObserver(InsiderApi::MessageTypes::EnumerateAudio, ""),
			m_Parent(parent),
			m_Owner(Owner) {

		{
			QList<QStandardItem*> cols;
			cols << (m_SoundParent = new QStandardItem("Sounds"));
			cols << new QStandardItem("");
			m_Parent->appendRow(cols);
			m_SoundParent->setData(QIcon(":/mgdt/icons/blue_music.png"), Qt::DecorationRole);
				}
		{
			QList<QStandardItem*> cols;
			cols << (m_MusicParent = new QStandardItem("Music"));
			cols << new QStandardItem("");
			m_Parent->appendRow(cols);
			m_MusicParent->setData(QIcon(":/mgdt/icons/blue_Play_music.png"), Qt::DecorationRole);
		}
	}

	HanderStatus Message(InsiderApi::InsiderMessageBuffer &message) override { 
		auto hdr = message.GetAndPull<InsiderApi::PayLoad_ListBase>();

#if 0
		struct PayLoad_AudioListItem {
			u16 Index;
			u8 unused;
			enum class AudioType : u8 {
				Unknown, Sound, Music,
			} Type;
			u16 NameLen;
			u16 ClassNameLen;
			char Name_Class[0];
		};
#endif // 0


		for (unsigned i = 0; i < hdr->Count; ++i) {
			auto *item = message.GetAndPull<InsiderApi::PayLoad_AudioListItem>();
			
			const char *Name = message.PullString();
			const char *Class = message.PullString();

			QList<QStandardItem*> cols;
			cols << new QStandardItem(Name);
			cols << new QStandardItem(Class);

			switch (item->Type) {
			case InsiderApi::PayLoad_AudioListItem::AudioType::Sound:
				m_SoundParent->appendRow(cols);
				break;		 
			case InsiderApi::PayLoad_AudioListItem::AudioType::Music:
				m_MusicParent->appendRow(cols);
				break;
			default:
				break;
			}
		}
		m_Owner->RequestFinished(this);
		m_Parent->sortChildren(0);
		m_SoundParent->sortChildren(0);
		m_MusicParent->sortChildren(0);
		return HanderStatus::Remove; 
	};
private:
	QStandardItem *m_Parent;
	QStandardItem *m_SoundParent;
	QStandardItem *m_MusicParent;
	AudioListWidget *m_Owner;
};

//-----------------------------------------
//-----------------------------------------

static ResourceEditorTabRegister::Register<AudioListWidget> _Register("AudioListWidget");

AudioListWidget::AudioListWidget(QWidget *parent)
	: ResourceEditorBaseTab(parent) {
	SetName("Audio");
	ui = new Ui::AudioListWidget();
	ui->setupUi(this);
	
	ui->treeView->setContextMenuPolicy(Qt::ActionsContextMenu);
	ui->treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui->treeView->setExpandsOnDoubleClick(false);

	connect(ui->actionPlay, SIGNAL(triggered()), SLOT(PlayAudioAction()));
	connect(ui->actionStop, SIGNAL(triggered()), SLOT(StopAudioAction()));
	connect(ui->actionCopy_path, SIGNAL(triggered()), SLOT(CopyPathAction()));
	connect(ui->treeView, SIGNAL(doubleClicked(const QModelIndex&)), SLOT(ItemDoubleClicked(const QModelIndex&)));

	ui->treeView->addAction(ui->actionPlay);
	ui->treeView->addAction(ui->actionStop);
	QAction *a = new QAction(this);
	a->setSeparator(true);
	ui->treeView->addAction(a);
	ui->treeView->addAction(ui->actionCopy_path);

	ResetTreeView();
}

AudioListWidget::~AudioListWidget() {
	ResetTreeView();
	delete ui;
}

//-----------------------------------------

void AudioListWidget::ResetTreeView() {
//	auto &settings = mgdtSettings::get();
    ui->treeView->setModel(nullptr);
	m_ViewModel = std::make_unique<QStandardItemModel>();

	m_ViewModel->setHorizontalHeaderItem(0, new QStandardItem("Name"));
	m_ViewModel->setHorizontalHeaderItem(1, new QStandardItem("Class"));

    ui->treeView->setModel(m_ViewModel.get());
	ui->treeView->setSelectionMode(QAbstractItemView::SingleSelection);
	  
	ui->treeView->setColumnWidth(0, 400);
	ui->treeView->setColumnWidth(1, 100);
}

//-----------------------------------------

void AudioListWidget::Refresh() {
	CancelRequests();
	ResetTreeView();

	QueueRequest(std::make_shared<LuaRequest>(m_ViewModel->invisibleRootItem(), this));
}

//-----------------------------------------

void AudioListWidget::PlayAudioAction() {
	//ui->treeView->se
	auto idx = ui->treeView->selectionModel()->currentIndex();
	auto parent = idx.parent();
	QString type = parent.data(Qt::DisplayRole).toString();
	if (type == "Sounds") {
		type = "Effect";
	} else 
		if (type == "Music") {
		} else
			return;
	
	QString str = "Inst.SoundEngine:Play" + type + "([[" +  idx.data(Qt::DisplayRole).toString() + "]])";
	GetRemoteConsole().ExecuteCode(str);
}

void AudioListWidget::StopAudioAction() {
	//ui->treeView->se
	auto idx = ui->treeView->selectionModel()->currentIndex();
	auto parent = idx.parent();
	QString type = parent.data(Qt::DisplayRole).toString();
	if (type == "Sounds") {
		return;
	} else
		if (type == "Music") {
		} else
			return;

	QString str = "Inst.SoundEngine:Stop" + type + "([[" + idx.data(Qt::DisplayRole).toString() + "]])";
	GetRemoteConsole().ExecuteCode(str);
}

void AudioListWidget::CopyPathAction() {
	//ui->treeView->se
	auto idx = ui->treeView->selectionModel()->currentIndex();
	QString str = idx.data(Qt::DisplayRole).toString();
	QClipboard *clipboard = QApplication::clipboard();
	clipboard->setText(str);
}

//-----------------------------------------

void AudioListWidget::ItemDoubleClicked(const QModelIndex& index) {
	PlayAudioAction();
}
