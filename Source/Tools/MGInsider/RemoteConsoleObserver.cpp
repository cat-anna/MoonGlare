/*
  * Generated by cppsrc.sh
  * On 2015-08-08 19:50:25,77
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#include PCH_HEADER
#include "RemoteConsole.h"
#include "Notifications.h"

using namespace MoonGlare::Debug::InsiderApi;

std::atomic<InsiderApi::u32> RemoteConsoleObserver::s_RequestCounter = 1;

RemoteConsoleObserver::RemoteConsoleObserver(InsiderApi::MessageTypes MessageType, RemoteConsoleRequestQueue *Queue):
		m_RequestId(++s_RequestCounter),
		m_MessageType(MessageType),
		m_Timeout(5 * 1000),
		m_Attempts(0),
		m_Queue(Queue),
		m_MaxAttempts(10) {
	m_Timer = std::make_unique<QTimer>(this);
	m_Timer->setSingleShot(true);
	connect(m_Timer.get(), SIGNAL(timeout()), SLOT(Expired()));
}

RemoteConsoleObserver::HanderStatus RemoteConsoleObserver::ProcessMessage(InsiderApi::InsiderMessageBuffer &message) {
	auto ret = Message(message);
	m_Queue->RequestFinished(this);
	return ret;
}


void RemoteConsoleObserver::SetRetriesCount(unsigned timeout, unsigned attempts) {
	m_Timeout = timeout;
	m_Attempts = attempts;
}

void RemoteConsoleObserver::OnSend() {
	m_Timer->start(m_Timeout);
}

void RemoteConsoleObserver::OnRecive() {
	m_Timer->stop();
}

RemoteConsoleObserver::TimeOutAction RemoteConsoleObserver::TimedOut() {
	++m_Attempts;
	if (m_Attempts > m_MaxAttempts)
		return TimeOutAction::Remove;
	else
		return TimeOutAction::Resend;
};

void RemoteConsoleObserver::Expired() {
	auto shared_this = shared_from_this();
	if (!shared_this)
		return;
	switch (TimedOut()) {
	case TimeOutAction::Remove:
		if (m_Queue)
			m_Queue->RequestFinished(this);
		OnError(ErrorStatus::TimedOut);
		GetRemoteConsole().RequestTimedout(shared_from_this());
		AddLogf(Warning, "Message timed-out %d (%s)", m_MessageType, typeid(*this).name());
		break;
	case TimeOutAction::Resend:
		AddLogf(Info, "Resending message %d (%s)", m_MessageType, typeid(*this).name());
		GetRemoteConsole().MakeRequest(shared_from_this());
		break;
	default:
		break;
	}
} 

//-----------------------------------------

void RemoteConsoleEnumerationObserver::BuildMessage(InsiderApi::InsiderMessageBuffer &buffer) {
	auto hdr = buffer.AllocAndZero<InsiderApi::PayLoad_EnumerateRequest>();
	hdr->PathLen = (InsiderApi::u16)m_Location.length() + 1;
	buffer.PushString(m_Location);
}

//-----------------------------------------
//-----------------------------------------
//-----------------------------------------

RemoteConsoleRequestQueue::RemoteConsoleRequestQueue() : m_QueueName() {
}

RemoteConsoleRequestQueue::~RemoteConsoleRequestQueue() {
	CancelRequests();
	if (!m_QueueName.empty())
		Notifications::SendRemoveStateValue(m_QueueName);
}

void RemoteConsoleRequestQueue::SetQueueName(const std::string &Name) {
	m_QueueName = std::string("Command queues/") + Name;
}

void RemoteConsoleRequestQueue::CancelRequests() {
	m_RequestList.clear();
}

void RemoteConsoleRequestQueue::Process() {
	if (m_RequestList.empty()) {
		if (!m_QueueName.empty())
			Notifications::SendRemoveStateValue(m_QueueName);
		return;
	} 

	if (!m_CurrentRequest) {
		m_CurrentRequest = m_RequestList.front();
		GetRemoteConsole().MakeRequest(std::static_pointer_cast<RemoteConsoleObserver>(m_CurrentRequest));
	}

	if (!m_QueueName.empty()) {
		char buf[256];
		sprintf_s(buf, "%d", m_RequestList.size());
		Notifications::SendSetStateValue(m_QueueName, buf);
	} 
}

void RemoteConsoleRequestQueue::RequestFinished(RemoteConsoleObserver *sender) {
	if (m_RequestList.empty())
		return;
	m_RequestList.remove_if([this, sender](const SharedLuaStateRequest & item) {
		if (m_CurrentRequest == item) {
			m_CurrentRequest = nullptr;
		}
		return item.get() == sender;
	});
	Process();
}

void RemoteConsoleRequestQueue::QueueRequest(SharedLuaStateRequest request) {
	request->SetQueueOwner(this);
	m_RequestList.push_back(request);
	Process();
	//AddLogf(Debug, "Queued message %d (%s) (queue: %p:%s)", request->GetMessageType(), typeid(*request.get()).name(), this, typeid(*this).name());
}
