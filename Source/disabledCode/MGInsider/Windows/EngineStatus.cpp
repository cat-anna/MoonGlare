#include PCH_HEADER
#include "mgdtSettings.h"
#include "ui_EngineStatus.h"
#include "EngineStatus.h"
#include <DockWindowInfo.h>
#include <Notifications.h>

class EngineInfoRequest : public RemoteConsoleObserver {
public:
	EngineInfoRequest(EngineStatus *Owner) : RemoteConsoleObserver(InsiderApi::MessageTypes::InfoRequest, Owner),
			m_Owner(Owner) {

		m_Owner->RemoveValue("Engine/Version");
		m_Owner->RemoveValue("Engine/ExeName");
		m_Owner->RemoveValue("Engine/BuildDate");
	}

	HanderStatus Message(InsiderApi::InsiderMessageBuffer &message) override {
		if (m_Queue)
			m_Queue->RequestFinished(this);
		message.GetAndPull<InsiderApi::PayLoad_InfoResponse>();

		auto ver = message.PullString();
		auto exen = message.PullString();
		auto buildd = message.PullString();

		m_Owner->SetValue("Engine/Version", ver);
		m_Owner->SetValue("Engine/ExeName", exen);
		m_Owner->SetValue("Engine/BuildDate", buildd);

		return HanderStatus::Remove;
	};
private:
	EngineStatus *m_Owner;
};

class EnginePingPong : public RemoteConsoleObserver {
public:
	enum class EngineState {
		Unknown, Active, Broken,
	};

	EnginePingPong(EngineStatus *Owner) : 
			RemoteConsoleObserver(InsiderApi::MessageTypes::Ping, Owner), 
			m_Owner(Owner) {
		ChangeState(EngineState::Broken);
	}

	HanderStatus Message(InsiderApi::InsiderMessageBuffer &message) override {
		m_Queue->RequestFinished(this);
		ChangeState(EngineState::Active);

		auto sthis = shared_from_this();
		QTimer::singleShot(m_Timeout, [sthis, this]() {
			m_Queue->QueueRequest(sthis);
		});

		return HanderStatus::Remove;
	};

	virtual void OnSend() override {
		RemoteConsoleObserver::OnSend();
		m_SendTime = std::chrono::steady_clock::now();
	}
	virtual void OnRecive() override {
		RemoteConsoleObserver::OnRecive();
		auto t = std::chrono::steady_clock::now();
		std::chrono::duration<double> sec = t - m_SendTime;
		char buf[64];
		sprintf_s(buf, "%d ms", static_cast<int>(sec.count() * 1000));
		m_Owner->SetValue("Ping", buf);
	}

	virtual TimeOutAction TimedOut() override {
		ChangeState(EngineState::Broken);
		return TimeOutAction::Resend;
	}

	void ChangeState(EngineState val) {
		if (m_EngineState == val)
			return;
		m_EngineState = val;

		if (m_EngineState == EngineState::Active) {
			m_Owner->SetValue("State", "Active", ":/mgdt/icons/blue_ok.png");
			m_Queue->QueueRequest(std::make_shared<EngineInfoRequest>(m_Owner));
			Notifications::SendOnEngineConnected();
		} else {
			m_Owner->SetValue("State", "Broken", ":/mgdt/icons/red_flag.png");
			m_Owner->RemoveValue("Ping");
			std::make_shared<EngineInfoRequest>(m_Owner);
			Notifications::SendOnEngineDisconnected();
		}
	}
private:
	EngineStatus *m_Owner;
	std::chrono::steady_clock::time_point m_SendTime;
	EngineState m_EngineState = EngineState::Unknown;
};

//-----------------------------------------
//-----------------------------------------
//-----------------------------------------

struct EngineStatusInfo : public QtShared::DockWindowInfo {
	virtual std::shared_ptr<QtShared::DockWindow> CreateInstance(QWidget *parent) override {
		return std::make_shared<EngineStatus>(parent);
	}

	EngineStatusInfo(QWidget *Parent) : QtShared::DockWindowInfo(Parent) {
		SetSettingID("EngineStatusInfo");
		SetDisplayName(tr("Engine status"));
		SetShortcut("F12");
	}
};
QtShared::DockWindowClassRgister::Register<EngineStatusInfo> EngineStatusInfoReg("EngineStatus");

EngineStatus::EngineStatus(QWidget *parent)
	: QtShared::DockWindow(parent)
{
	SetSettingID("EngineStatus");
	m_Ui = std::make_unique<Ui::EngineStatus>();
	m_Ui->setupUi(this);

	connect(Notifications::Get(), SIGNAL(SetStateValue(const std::string&, const std::string &, const std::string&)), SLOT(SetValue(const std::string&, const std::string &, const std::string&)));
	connect(Notifications::Get(), SIGNAL(RemoveStateValue(const std::string&)), SLOT(RemoveValue(const std::string&)));

	m_ViewModel = std::make_unique<QStandardItemModel>();
	m_ViewModel->setHorizontalHeaderItem(0, new QStandardItem("Parameter"));
	m_ViewModel->setHorizontalHeaderItem(1, new QStandardItem("Value"));

	m_Ui->treeView->setModel(m_ViewModel.get());
	m_Ui->treeView->setSelectionMode(QAbstractItemView::SingleSelection);

	m_Ui->treeView->setColumnWidth(0, 150);
	m_Ui->treeView->setColumnWidth(1, 100);

	SetQueueName("Engine status");
	QueueRequest(std::make_shared<EnginePingPong>(this));
}

EngineStatus::~EngineStatus() {
	m_ViewModel.reset();
	m_Ui.reset();
}

bool EngineStatus::DoSaveSettings(pugi::xml_node node) const {
	DockWindow::DoSaveSettings(node);
//	SaveState(node, m_Ui->splitter, "Splitter:State");
	return true;
}

bool EngineStatus::DoLoadSettings(const pugi::xml_node node) {
	DockWindow::DoLoadSettings(node);
//	LoadState(node, m_Ui->splitter, "Splitter:State");
	return true;
}

void EngineStatus::SetValue(const std::string& ID, const std::string &Value, const std::string& Icon) {
	if (!m_ViewModel)
		return;
	auto root = m_ViewModel->invisibleRootItem();
	auto setitem = [&](QStandardItem* item) {
		auto row = item->row();
		auto itemroot = item->parent() ? item->parent() : root;
		itemroot->child(row, 0)->setData(QIcon(QString(Icon.c_str())), Qt::DecorationRole);
		itemroot->child(row, 1)->setData(QString(Value.c_str()), Qt::DisplayRole);
	};
	auto it = m_Values.find(ID);

	if (it != m_Values.end()) {
		setitem(it->second);
		return;
	}

	QStandardItem *item;
	QStandardItem *group = root;
	std::string Name;

	auto pos = ID.find('/');
	if (pos == std::string::npos) {
		Name = ID;
	} else {
		std::string groupname = ID.substr(0, pos);
		Name = ID.substr(pos + 1);

		auto gropupit = m_Values.find(groupname);

		if (gropupit == m_Values.end()) {
			QList<QStandardItem*> cols;
			cols << (item = new QStandardItem(groupname.c_str()));
			cols << new QStandardItem("");
			root->appendRow(cols);
			group = item;
			m_Values[groupname] = item;
		} else {
			group = gropupit->second;
		}
	}

	QList<QStandardItem*> cols;
	cols << (item = new QStandardItem(Name.c_str()));
	cols << new QStandardItem("");
	group->appendRow(cols);
	setitem(item);

	m_ViewModel->sort(0);

	m_Values[ID] = item;
}

void EngineStatus::RemoveValue(const std::string& ID) {
	if (!m_ViewModel)
		return;

	auto it = m_Values.find(ID);
	if (it == m_Values.end())
		return;

	auto parent = it->second->parent();
	if (!parent)
		parent = m_ViewModel->invisibleRootItem();
	parent->removeRow(it->second->row());
	m_Values.erase(it);
}

//-----------------------------------------
