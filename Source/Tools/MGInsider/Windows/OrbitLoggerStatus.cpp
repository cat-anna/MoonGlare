#include PCH_HEADER
#include "mgdtSettings.h"
#include "ui_OrbitLoggerStatus.h"
#include "OrbitLoggerStatus.h"
#include <DockWindowInfo.h>
#include <Notifications.h>

class OrbitLoggerStatusRequest : public RemoteConsoleObserver {
public:
	OrbitLoggerStatusRequest(OrbitLoggerStatus *Owner) : RemoteConsoleObserver(InsiderApi::MessageTypes::OrbitLoggerStateRequest, Owner),
			m_Owner(Owner) {

	}

	HanderStatus Message(InsiderApi::InsiderMessageBuffer &message) override {
		auto ptr = message.GetAndPull<InsiderApi::PayLoad_OrbitLoggerStateResponse>();

		m_Owner->ShowData(*ptr);

		return HanderStatus::Remove;
	};
private:
	OrbitLoggerStatus *m_Owner;
};

//-----------------------------------------
//-----------------------------------------
//-----------------------------------------

struct OrbitLoggerStatusInfo : public QtShared::DockWindowInfo {
	virtual std::shared_ptr<QtShared::DockWindow> CreateInstance(QWidget *parent) override {
		return std::make_shared<OrbitLoggerStatus>(parent);
	}

	OrbitLoggerStatusInfo(QWidget *Parent) : QtShared::DockWindowInfo(Parent) {
		SetSettingID("OrbitLoggerStatusInfo");
		SetDisplayName(tr("OrbitLogger status"));
		SetShortcut("F11");
	}
};
QtShared::DockWindowClassRgister::Register<OrbitLoggerStatusInfo> OrbitLoggerStatusInfoReg("OrbitLoggerStatus");

OrbitLoggerStatus::OrbitLoggerStatus(QWidget *parent)
	: QtShared::DockWindow(parent)
{
	SetSettingID("OrbitLoggerStatus");
	SetQueueName("OrbitLoggerStatus");
	m_Ui = std::make_unique<Ui::OrbitLoggerStatus>();
	m_Ui->setupUi(this);

	m_ViewModel = std::make_unique<QStandardItemModel>();
	m_ViewModel->setHorizontalHeaderItem(0, new QStandardItem("Index"));
	m_ViewModel->setHorizontalHeaderItem(1, new QStandardItem("Name"));
	m_ViewModel->setHorizontalHeaderItem(2, new QStandardItem("Enabled"));
	m_ViewModel->setHorizontalHeaderItem(3, new QStandardItem("Lines"));

	m_Ui->treeView->setModel(m_ViewModel.get());
	m_Ui->treeView->setSelectionMode(QAbstractItemView::SingleSelection);
	m_Ui->treeView->setColumnWidth(0, 50);
	m_Ui->treeView->setColumnWidth(1, 100);
	m_Ui->treeView->setColumnWidth(2, 70);
	m_Ui->treeView->setColumnWidth(3, 50);
	
	SetQueueName("Engine status");
	SetAutoRefresh(true);
}

OrbitLoggerStatus::~OrbitLoggerStatus() {
	m_ViewModel.reset();
	m_Ui.reset();
}

bool OrbitLoggerStatus::DoSaveSettings(pugi::xml_node node) const {
	QtShared::DockWindow::DoSaveSettings(node);
	return true;
}

bool OrbitLoggerStatus::DoLoadSettings(const pugi::xml_node node) {
	QtShared::DockWindow::DoLoadSettings(node);
	return true;
}

//-----------------------------------------

void OrbitLoggerStatus::Refresh() {
	QueueRequest(std::make_shared<OrbitLoggerStatusRequest>(this));
}

void OrbitLoggerStatus::ShowData(const InsiderApi::PayLoad_OrbitLoggerStateResponse &data) {
	m_ViewModel->removeRows(0, m_ViewModel->rowCount());

	for (size_t i = 0; i < OrbitLogger::LogChannels::MaxLogChannels; ++i) {
		auto &item = data.m_Table[i];
		QList<QStandardItem*> cols;

		cols << new QStandardItem(std::to_string(i).c_str());
		cols << new QStandardItem(item.Name);
		cols << new QStandardItem(item.Enabled ? "Enabled" : "Disabled");
		cols << new QStandardItem(std::to_string(item.LinesPushed).c_str());

		m_ViewModel->invisibleRootItem()->appendRow(cols);
	}
}

//-----------------------------------------
